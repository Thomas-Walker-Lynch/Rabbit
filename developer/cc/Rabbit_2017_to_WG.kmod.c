// rabbit_2017_to_WG.c — forward UID 2017 traffic to WG

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/atomic.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <net/sock.h>
#include <net/inet_sock.h>

#define RABBIT_UID 2017
#define DEV_NAME   "US"   /* WireGuard iface to force */

static atomic64_t cnt_v4_local_out = ATOMIC64_INIT(0);
static atomic64_t cnt_v4_postroute = ATOMIC64_INIT(0);

struct v4_cidr { __be32 net, mask; };
#define V4(x) (__force __be32)cpu_to_be32(x)
static struct v4_cidr v4_excepts[] = {
  { V4(0x7f000000), V4(0xff000000) }, /* 127.0.0.0/8 */
  { V4(0xa9fe0000), V4(0xffff0000) }, /* 169.254.0.0/16 */
  { V4(0xc0a80000), V4(0xffff0000) }, /* 192.168.0.0/16  (adjust) */
  { V4(0x0a000002), V4(0xffffffff) }, /* 10.0.0.2/32    US local */
  { V4(0x0a080004), V4(0xffffffff) }, /* 10.8.0.4/32    x6 local */
};
static inline bool v4_in_cidr(__be32 a, struct v4_cidr c){ return (a & c.mask) == (c.net & c.mask); }
static bool v4_is_except(__be32 d){ int i; for (i=0;i<ARRAY_SIZE(v4_excepts);i++) if (v4_in_cidr(d, v4_excepts[i])) return true; return false; }

static int us_ifindex;  /* cached ifindex for DEV_NAME */

static inline bool from_uid_2017(const struct nf_hook_state *st, struct sk_buff *skb){
  struct sock *sk = st->sk ? st->sk : skb_to_full_sk(skb);
  if (!sk) return false;
  return __kuid_val(sock_i_uid(sk)) == RABBIT_UID;
}

/* Bind the socket to DEV_NAME once we see its first packet */
static inline void maybe_bind_socket_to_us(struct sk_buff *skb, const struct iphdr *iph, const struct nf_hook_state *st){
  struct sock *sk = st->sk ? st->sk : skb_to_full_sk(skb);
  if (!sk || !us_ifindex) return;
  if (v4_is_except(iph->daddr)) return;
  if (READ_ONCE(sk->sk_bound_dev_if) == us_ifindex) return;
  WRITE_ONCE(sk->sk_bound_dev_if, us_ifindex);
}

static unsigned int rabbit_v4_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *st){
  if (st->pf != NFPROTO_IPV4) return NF_ACCEPT;

  if (st->hook == NF_INET_LOCAL_OUT) {
    if (from_uid_2017(st, skb)) {
      const struct iphdr *iph = ip_hdr(skb);
      if (iph) maybe_bind_socket_to_us(skb, iph, st);
      atomic64_inc(&cnt_v4_local_out);
    }
  } else if (st->hook == NF_INET_POST_ROUTING) {
    if (from_uid_2017(st, skb)) atomic64_inc(&cnt_v4_postroute);
  }
  return NF_ACCEPT;
}

static struct nf_hook_ops rabbit_ops[] = {
  { .hook = rabbit_v4_hook, .pf = NFPROTO_IPV4, .hooknum = NF_INET_LOCAL_OUT,    .priority = NF_IP_PRI_FIRST },
  { .hook = rabbit_v4_hook, .pf = NFPROTO_IPV4, .hooknum = NF_INET_POST_ROUTING, .priority = NF_IP_PRI_FIRST },
};

static int __init rabbit_init(void){
  struct net_device *dev;
  rcu_read_lock();
  dev = dev_get_by_name_rcu(&init_net, DEV_NAME);
  us_ifindex = dev ? dev->ifindex : 0;
  rcu_read_unlock();
  if (!us_ifindex) {
    pr_err("rabbit_uid2017_bind: device \"%s\" not found\n", DEV_NAME);
    return -ENODEV;
  }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
  {
    int ret = nf_register_net_hooks(&init_net, rabbit_ops, ARRAY_SIZE(rabbit_ops));
    if (ret) return ret;
  }
#else
  { int ret = nf_register_hooks(rabbit_ops, ARRAY_SIZE(rabbit_ops)); if (ret) return ret; }
#endif
  pr_info("rabbit_uid2017_bind: loaded; UID=%d bound to dev %s(ifindex=%d)\n", RABBIT_UID, DEV_NAME, us_ifindex);
  return 0;
}

static void __exit rabbit_exit(void){
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
  nf_unregister_net_hooks(&init_net, rabbit_ops, ARRAY_SIZE(rabbit_ops));
#else
  nf_unregister_hooks(rabbit_ops, ARRAY_SIZE(rabbit_ops));
#endif
  pr_info("rabbit_uid2017_bind: unload v4(lo=%lld,po=%lld)\n",
          (long long)atomic64_read(&cnt_v4_local_out),
          (long long)atomic64_read(&cnt_v4_postroute));
}

module_init(rabbit_init);
module_exit(rabbit_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rabbit");
MODULE_DESCRIPTION("Force UID 2017 traffic onto dev \"US\" at LOCAL_OUT with IPv4 exceptions");
