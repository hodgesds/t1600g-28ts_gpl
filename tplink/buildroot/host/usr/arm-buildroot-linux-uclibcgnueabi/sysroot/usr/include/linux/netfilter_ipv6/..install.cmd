cmd_/Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/netfilter_ipv6/.install := perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/linux/netfilter_ipv6 /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/netfilter_ipv6 arm ip6_tables.h ip6t_HL.h ip6t_LOG.h ip6t_REJECT.h ip6t_ah.h ip6t_frag.h ip6t_hl.h ip6t_ipv6header.h ip6t_mh.h ip6t_opts.h ip6t_rt.h; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/linux/netfilter_ipv6 /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/netfilter_ipv6 arm ; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/generated/linux/netfilter_ipv6 /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/netfilter_ipv6 arm ; for F in ; do echo "\#include <asm-generic/$$F>" > /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/netfilter_ipv6/$$F; done; touch /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/netfilter_ipv6/.install