// -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 8 -*-
// rabbit_2017_to_WG.c — force UID 2017 traffic via WireGuard dev "US" (IPv4)
// Debuggable build with versioning and no-leak behavior.

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/atomic.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
#include <linux/rcupdate.h>
#include <linux/inetdevice.h>  /* __in_dev_get_rcu */
#include <net/sock.h>
#include <net/inet_sock.h>
#include <net/route.h>      /* ip_route_output_flow */
#include <net/ip.h>         /* RT_TOS() */
#include <net/dst.h>       /* sk_dst_set() / dst APIs */

#define RABBIT_WG_VERSION "1.2.2-2025-10-30"

#define US_UID   2017
#define DEV_NAME "US"       /* WireGuard iface to force */

/* dbg param (echo 1 > /sys/module/Rabbit_2017_to_WG/parameters/dbg) */
static bool dbg = true;
module_param(dbg ,bool ,0644);
MODULE_PARM_DESC(dbg ,"Enable debug logs (default: true)");

#define DBG(fmt, ...) do { \
  if (dbg) pr_info_ratelimited("rabbit_uid2017: " fmt ,##__VA_ARGS__); \
} while (0)

static atomic64_t cnt_v4_local_out = ATOMIC64_INIT(0);
static atomic64_t cnt_v4_postroute = ATOMIC64_INIT(0);
static atomic64_t cnt_drop_no_us   = ATOMIC64_INIT(0);
static atomic64_t cnt_drop_route   = ATOMIC64_INIT(0);

struct v4_cidr { __be32 net, mask; };
#define V4(x) (__force __be32)cpu_to_be32(x)

/* Exception destinations: never force to US. Adjust to taste. */
static struct v4_cidr v4_excepts[] = {
  { V4(0x7f000000) ,V4(0xff000000) } /* 127.0.0.0/8 (loopback) */
  ,{ V4(0xa9fe0000) ,V4(0xffff0000) } /* 169.254.0.0/16 (LLA)   */
  ,{ V4(0xc0a80000) ,V4(0xffff0000) } /* 192.168.0.0/16 (LAN)   */
  ,{ V4(0x0a000002) ,V4(0xffffffff) } /* 10.0.0.2/32    US local */
  ,{ V4(0x0a080004) ,V4(0xffffffff) } /* 10.8.0.4/32    x6 local */
};

static inline bool v4_in_cidr(__be32 a ,struct v4_cidr c) {
  return (a & c.mask) == (c.net & c.mask);
}
static bool v4_is_except(__be32 d) {
  int i;
  for (i = 0; i < ARRAY_SIZE(v4_excepts); i++)
    if (v4_in_cidr(d ,v4_excepts[i]))
      return true;
  return false;
}

static int us_ifindex;  /* cached ifindex for DEV_NAME */

static bool us_has_ipv4_addr(void) {
  bool has = false;
  rcu_read_lock();
  {
    struct net_device *dev = dev_get_by_index_rcu(&init_net ,us_ifindex);
    if (dev) {
      struct in_device *in_dev = __in_dev_get_rcu(dev);
      if (in_dev) {
        struct in_ifaddr *ifa;
        for (ifa = rcu_dereference(in_dev->ifa_list); ifa; ifa = rcu_dereference(ifa->ifa_next)) {
          if (ifa->ifa_address) { has = true; break; }
        }
      }
    }
  }
  rcu_read_unlock();
  return has;
}

static inline bool from_uid_2017(const struct nf_hook_state *st ,struct sk_buff *skb) {
  struct sock *sk = st->sk ? st->sk : skb_to_full_sk(skb);
  if (!sk) return false;
  return __kuid_val(sock_i_uid(sk)) == US_UID;
}

/* Ensure US device exists and is up (oper up). */
static bool us_dev_ok(void) {
  bool ok = false;
  rcu_read_lock();
  {
    struct net_device *dev = dev_get_by_index_rcu(&init_net ,us_ifindex);
    if (dev && netif_running(dev) && netif_oper_up(dev))
      ok = true;
  }
  rcu_read_unlock();
  return ok;
}

/* Attach a route (dst) to skb forcing oif=US so WG can encapsulate/send. */
/* helper: return first IPv4 address on the US device (net byte order) or 0 */
static __be32 us_first_ipv4(void)
{
  __be32 addr = 0;
  rcu_read_lock();
  {
    struct net_device *dev = dev_get_by_index_rcu(&init_net, us_ifindex);
    if (dev) {
      struct in_device *in_dev = __in_dev_get_rcu(dev);
      if (in_dev) {
        struct in_ifaddr *ifa;
        for (ifa = rcu_dereference(in_dev->ifa_list); ifa; ifa = rcu_dereference(ifa->ifa_next)) {
          if (ifa->ifa_address) { addr = ifa->ifa_address; break; }
        }
      }
    }
  }
  rcu_read_unlock();
  return addr;
}

/* Attach a route (dst) to skb forcing oif=US so WG can encapsulate/send. */
static bool ensure_route_to_us(struct sk_buff *skb ,const struct iphdr *iph)
{
  struct rtable *rt;
  __be32 saddr_us = us_first_ipv4(); /* may be 0 */

  struct flowi4 fl4 = {
    .daddr        = iph->daddr,
    .saddr        = saddr_us ? saddr_us : 0,   /* prefer US IP as source */
    .flowi4_tos   = RT_TOS(iph->tos),
    .flowi4_oif   = us_ifindex,                /* force egress on US */
    .flowi4_mark  = skb->mark,
    .flowi4_proto = iph->protocol,
  };

  /* If a dst is already set to US, keep it. */
  if (skb_dst(skb)) {
    const struct dst_entry *dst = skb_dst(skb);
    const struct net_device *out = dst ? dst->dev : NULL;
    if (out && out->ifindex == us_ifindex) {
      DBG("dst already US(ifindex=%d)\n", us_ifindex);
      return true;
    }
    DBG("replacing existing dst dev(ifindex=%d) with US(ifindex=%d)\n",
        out ? out->ifindex : -1, us_ifindex);
  }

  rt = ip_route_output_flow(&init_net, &fl4, NULL);
  if (IS_ERR(rt)) {
    DBG("ip_route_output_flow failed: daddr=%pI4 tos=0x%x oif=%d err=%ld\n",
        &iph->daddr, iph->tos, us_ifindex, PTR_ERR(rt));
    return false;
  }

  /* Log out_dev before transferring ownership to the skb */
  DBG("dst attached: daddr=%pI4 tos=0x%x oif=US(ifindex=%d) out_dev=%s\n",
      &iph->daddr, iph->tos, us_ifindex,
      rt->dst.dev ? rt->dst.dev->name : "(null)");

  /* Transfer reference to skb and stop using rt afterwards */
  skb_dst_drop(skb);
  skb_dst_set(skb, &rt->dst);  /* ownership moves to skb */

  return true;
}


/* Bind the socket to US; idempotent. */
static inline void bind_socket_to_us(struct sk_buff *skb ,const struct nf_hook_state *st) {
  struct sock *sk = st->sk ? st->sk : skb_to_full_sk(skb);
  if (sk && READ_ONCE(sk->sk_bound_dev_if) != us_ifindex) {
    WRITE_ONCE(sk->sk_bound_dev_if ,us_ifindex);
    DBG("sk_bound_dev_if <- US(ifindex=%d)\n" ,us_ifindex);
  }
}

static unsigned int rabbit_v4_hook(void *priv ,struct sk_buff *skb ,const struct nf_hook_state *st) {
  if (st->pf != NFPROTO_IPV4)
    return NF_ACCEPT;

  if (st->hook == NF_INET_LOCAL_OUT) {
    if (from_uid_2017(st ,skb)) {
      const struct iphdr *iph = ip_hdr(skb);
      if (iph) {
        DBG("LOCAL_OUT uid=2017 daddr=%pI4 tos=0x%x proto=%u\n"
            ,&iph->daddr ,iph->tos ,iph->protocol);

        if (!v4_is_except(iph->daddr)) {
          if (likely(us_dev_ok())) {
            if (likely(ensure_route_to_us(skb ,iph))) {
              bind_socket_to_us(skb ,st);
            } else {
              atomic64_inc(&cnt_drop_route);
              DBG("DROP: no route via US\n");
              return NF_DROP; /* no leaks */
            }
          } else {
            atomic64_inc(&cnt_drop_no_us);
            DBG("DROP: US not up\n");
            return NF_DROP;   /* no leaks */
          }
        } else {
          DBG("EXCEPT dst=%pI4 — not forcing US\n" ,&iph->daddr);
        }
      }
      atomic64_inc(&cnt_v4_local_out);
    }
  } else if (st->hook == NF_INET_POST_ROUTING) {
    if (from_uid_2017(st ,skb))
      atomic64_inc(&cnt_v4_postroute);
  }
  return NF_ACCEPT;
}

static struct nf_hook_ops rabbit_ops[] = {
  { .hook = rabbit_v4_hook , .pf = NFPROTO_IPV4 , .hooknum = NF_INET_LOCAL_OUT    , .priority = NF_IP_PRI_FIRST }
  ,{ .hook = rabbit_v4_hook , .pf = NFPROTO_IPV4 , .hooknum = NF_INET_POST_ROUTING , .priority = NF_IP_PRI_FIRST }
};

static int __init rabbit_init(void) {
  struct net_device *dev;

  rcu_read_lock();
  dev = dev_get_by_name_rcu(&init_net ,DEV_NAME);
  us_ifindex = dev ? dev->ifindex : 0;
  rcu_read_unlock();

  if (!us_ifindex) {
    pr_err("rabbit_uid2017_bind[%s]: device \"%s\" not found\n" ,RABBIT_WG_VERSION ,DEV_NAME);
    return -ENODEV;
  }

  if (!us_has_ipv4_addr())
    pr_warn("rabbit_uid2017_bind[%s]: US has no IPv4 address; inner flows may stall\n"
            ,RABBIT_WG_VERSION);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
  {
    int ret = nf_register_net_hooks(&init_net ,rabbit_ops ,ARRAY_SIZE(rabbit_ops));
    if (ret) return ret;
  }
#else
  {
    int ret = nf_register_hooks(rabbit_ops ,ARRAY_SIZE(rabbit_ops));
    if (ret) return ret;
  }
#endif
  pr_info("rabbit_uid2017_bind: loaded v%s; UID=%d bound to dev %s(ifindex=%d)\n"
          ,RABBIT_WG_VERSION ,US_UID ,DEV_NAME ,us_ifindex);
  return 0;
}

static void __exit rabbit_exit(void) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
  nf_unregister_net_hooks(&init_net ,rabbit_ops ,ARRAY_SIZE(rabbit_ops));
#else
  nf_unregister_hooks(rabbit_ops ,ARRAY_SIZE(rabbit_ops));
#endif
  pr_info("rabbit_uid2017_bind: unload v%s v4(lo=%lld,po=%lld,drop_no_us=%lld,drop_no_route=%lld)\n"
          ,RABBIT_WG_VERSION
          ,(long long)atomic64_read(&cnt_v4_local_out)
          ,(long long)atomic64_read(&cnt_v4_postroute)
          ,(long long)atomic64_read(&cnt_drop_no_us)
          ,(long long)atomic64_read(&cnt_drop_route));
}

module_init(rabbit_init);
module_exit(rabbit_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rabbit");
MODULE_DESCRIPTION("Force UID 2017 traffic onto dev \"US\" at LOCAL_OUT with IPv4 exceptions and attached dst");
MODULE_VERSION(RABBIT_WG_VERSION);
