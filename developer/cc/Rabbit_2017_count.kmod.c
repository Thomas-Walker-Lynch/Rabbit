// rabbit_uid2017.c — Count v4/v6 LOCAL_OUT & POST_ROUTING packets from UID 2017 only.

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/atomic.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <net/sock.h>
#include <net/inet_sock.h>

/* Hardcoded target UID */
#define RABBIT_UID 2017

/* Counters: only increment when packet is from UID 2017 */
static atomic64_t cnt_v4_local_out  = ATOMIC64_INIT(0);
static atomic64_t cnt_v4_postroute  = ATOMIC64_INIT(0);
static atomic64_t cnt_v6_local_out  = ATOMIC64_INIT(0);
static atomic64_t cnt_v6_postroute  = ATOMIC64_INIT(0);

static inline bool from_uid_2017(const struct nf_hook_state *st, struct sk_buff *skb) {
  struct sock *sk = st->sk ? st->sk : skb_to_full_sk(skb);
  if (!sk) return false;                       /* no socket context */
  return __kuid_val(sock_i_uid(sk)) == RABBIT_UID;
}

static unsigned int rabbit_v4_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *st) {
  if (!from_uid_2017(st, skb)) return NF_ACCEPT;

  if (st->hook == NF_INET_LOCAL_OUT)          atomic64_inc(&cnt_v4_local_out);
  else if (st->hook == NF_INET_POST_ROUTING)  atomic64_inc(&cnt_v4_postroute);
  return NF_ACCEPT;
}

static unsigned int rabbit_v6_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *st) {
  if (!from_uid_2017(st, skb)) return NF_ACCEPT;

  if (st->hook == NF_INET_LOCAL_OUT)          atomic64_inc(&cnt_v6_local_out);
  else if (st->hook == NF_INET_POST_ROUTING)  atomic64_inc(&cnt_v6_postroute);
  return NF_ACCEPT;
}

static struct nf_hook_ops rabbit_ops[] = {
  { .hook = rabbit_v4_hook, .pf = NFPROTO_IPV4, .hooknum = NF_INET_LOCAL_OUT,    .priority = NF_IP_PRI_FIRST },
  { .hook = rabbit_v4_hook, .pf = NFPROTO_IPV4, .hooknum = NF_INET_POST_ROUTING, .priority = NF_IP_PRI_FIRST },
  { .hook = rabbit_v6_hook, .pf = NFPROTO_IPV6, .hooknum = NF_INET_LOCAL_OUT,    .priority = NF_IP6_PRI_FIRST },
  { .hook = rabbit_v6_hook, .pf = NFPROTO_IPV6, .hooknum = NF_INET_POST_ROUTING, .priority = NF_IP6_PRI_FIRST },
};

static int __init rabbit_init(void) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
  int ret = nf_register_net_hooks(&init_net, rabbit_ops, ARRAY_SIZE(rabbit_ops));
#else
  int ret = nf_register_hooks(rabbit_ops, ARRAY_SIZE(rabbit_ops));
#endif
  if (ret) pr_err("rabbit_uid2017: nf_register_* failed: %d\n", ret);
  else     pr_info("rabbit_uid2017: loaded; counting UID %d only (no-op)\n", RABBIT_UID);
  return ret;
}

static void __exit rabbit_exit(void) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
  nf_unregister_net_hooks(&init_net, rabbit_ops, ARRAY_SIZE(rabbit_ops));
#else
  nf_unregister_hooks(rabbit_ops, ARRAY_SIZE(rabbit_ops));
#endif
  pr_info("rabbit_uid2017: unload stats v4(lo=%lld,po=%lld) v6(lo=%lld,po=%lld) [UID=%d]\n",
          (long long)atomic64_read(&cnt_v4_local_out),
          (long long)atomic64_read(&cnt_v4_postroute),
          (long long)atomic64_read(&cnt_v6_local_out),
          (long long)atomic64_read(&cnt_v6_postroute),
          RABBIT_UID);
}

module_init(rabbit_init);
module_exit(rabbit_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rabbit");
MODULE_DESCRIPTION("Count LOCAL_OUT/POST_ROUTING packets from UID 2017");
