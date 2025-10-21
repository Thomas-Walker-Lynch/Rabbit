// rabbit_noop.c — Rabbit no-op netfilter interposer (Debian 12/Bookworm)
// Build: out-of-tree module. Load/unload to verify hook coverage.
// Behavior: increments counters, returns NF_ACCEPT. No packet mutation.

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/atomic.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>

static atomic64_t cnt_v4_local_out  = ATOMIC_LONG_INIT(0);
static atomic64_t cnt_v4_postroute  = ATOMIC_LONG_INIT(0);
static atomic64_t cnt_v6_local_out  = ATOMIC_LONG_INIT(0);
static atomic64_t cnt_v6_postroute  = ATOMIC_LONG_INIT(0);

static unsigned int rabbit_v4_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *st) {
  if (st->hook == NF_INET_LOCAL_OUT)     atomic64_inc(&cnt_v4_local_out);
  else if (st->hook == NF_INET_POST_ROUTING) atomic64_inc(&cnt_v4_postroute);
  return NF_ACCEPT;
}

static unsigned int rabbit_v6_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *st) {
  if (st->hook == NF_INET_LOCAL_OUT)     atomic64_inc(&cnt_v6_local_out);
  else if (st->hook == NF_INET_POST_ROUTING) atomic64_inc(&cnt_v6_postroute);
  return NF_ACCEPT;
}

static struct nf_hook_ops rabbit_ops[] = {
  { .hook = rabbit_v4_hook, .pf = NFPROTO_IPV4, .hooknum = NF_INET_LOCAL_OUT,   .priority = NF_IP_PRI_FIRST },
  { .hook = rabbit_v4_hook, .pf = NFPROTO_IPV4, .hooknum = NF_INET_POST_ROUTING,.priority = NF_IP_PRI_FIRST },
  { .hook = rabbit_v6_hook, .pf = NFPROTO_IPV6, .hooknum = NF_INET_LOCAL_OUT,   .priority = NF_IP6_PRI_FIRST },
  { .hook = rabbit_v6_hook, .pf = NFPROTO_IPV6, .hooknum = NF_INET_POST_ROUTING,.priority = NF_IP6_PRI_FIRST },
};

static int __init rabbit_init(void) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
  int ret = nf_register_net_hooks(&init_net, rabbit_ops, ARRAY_SIZE(rabbit_ops));
#else
  int ret = nf_register_hooks(rabbit_ops, ARRAY_SIZE(rabbit_ops));
#endif
  if (ret)
    pr_err("rabbit_noop: nf_register_* failed: %d\n", ret);
  else
    pr_info("rabbit_noop: loaded (no-op)\n");
  return ret;
}

static void __exit rabbit_exit(void) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
  nf_unregister_net_hooks(&init_net, rabbit_ops, ARRAY_SIZE(rabbit_ops));
#else
  nf_unregister_hooks(rabbit_ops, ARRAY_SIZE(rabbit_ops));
#endif
  pr_info("rabbit_noop: unload stats v4(lo=%lld,po=%lld) v6(lo=%lld,po=%lld)\n",
          (long long)atomic64_read(&cnt_v4_local_out),
          (long long)atomic64_read(&cnt_v4_postroute),
          (long long)atomic64_read(&cnt_v6_local_out),
          (long long)atomic64_read(&cnt_v6_postroute));
}

module_init(rabbit_init);
module_exit(rabbit_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rabbit");
MODULE_DESCRIPTION("Rabbit no-op netfilter interposer");
