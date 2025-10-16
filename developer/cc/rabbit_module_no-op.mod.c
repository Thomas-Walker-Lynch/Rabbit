// rabit_noop.c — Rabit no-op netfilter interposer (Debian 12/Bookworm)
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

static unsigned int rabit_v4_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *st) {
  if (st->hook == NF_INET_LOCAL_OUT)     atomic64_inc(&cnt_v4_local_out);
  else if (st->hook == NF_INET_POST_ROUTING) atomic64_inc(&cnt_v4_postroute);
  return NF_ACCEPT;
}

static unsigned int rabit_v6_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *st) {
  if (st->hook == NF_INET_LOCAL_OUT)     atomic64_inc(&cnt_v6_local_out);
  else if (st->hook == NF_INET_POST_ROUTING) atomic64_inc(&cnt_v6_postroute);
  return NF_ACCEPT;
}

static struct nf_hook_ops rabit_ops[] = {
  { .hook = rabit_v4_hook, .pf = NFPROTO_IPV4, .hooknum = NF_INET_LOCAL_OUT,   .priority = NF_IP_PRI_FIRST },
  { .hook = rabit_v4_hook, .pf = NFPROTO_IPV4, .hooknum = NF_INET_POST_ROUTING,.priority = NF_IP_PRI_FIRST },
  { .hook = rabit_v6_hook, .pf = NFPROTO_IPV6, .hooknum = NF_INET_LOCAL_OUT,   .priority = NF_IP6_PRI_FIRST },
  { .hook = rabit_v6_hook, .pf = NFPROTO_IPV6, .hooknum = NF_INET_POST_ROUTING,.priority = NF_IP6_PRI_FIRST },
};

static int __init rabit_init(void) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
  int ret = nf_register_net_hooks(&init_net, rabit_ops, ARRAY_SIZE(rabit_ops));
#else
  int ret = nf_register_hooks(rabit_ops, ARRAY_SIZE(rabit_ops));
#endif
  if (ret)
    pr_err("rabit_noop: nf_register_* failed: %d\n", ret);
  else
    pr_info("rabit_noop: loaded (no-op)\n");
  return ret;
}

static void __exit rabit_exit(void) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
  nf_unregister_net_hooks(&init_net, rabit_ops, ARRAY_SIZE(rabit_ops));
#else
  nf_unregister_hooks(rabit_ops, ARRAY_SIZE(rabit_ops));
#endif
  pr_info("rabit_noop: unload stats v4(lo=%lld,po=%lld) v6(lo=%lld,po=%lld)\n",
          (long long)atomic64_read(&cnt_v4_local_out),
          (long long)atomic64_read(&cnt_v4_postroute),
          (long long)atomic64_read(&cnt_v6_local_out),
          (long long)atomic64_read(&cnt_v6_postroute));
}

module_init(rabit_init);
module_exit(rabit_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rabit");
MODULE_DESCRIPTION("Rabit no-op netfilter interposer");
