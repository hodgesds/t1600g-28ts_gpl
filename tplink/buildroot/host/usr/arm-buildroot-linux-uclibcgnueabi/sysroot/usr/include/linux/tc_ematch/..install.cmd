cmd_/Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/tc_ematch/.install := perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/linux/tc_ematch /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/tc_ematch arm tc_em_cmp.h tc_em_meta.h tc_em_nbyte.h tc_em_text.h; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/linux/tc_ematch /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/tc_ematch arm ; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/generated/linux/tc_ematch /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/tc_ematch arm ; for F in ; do echo "\#include <asm-generic/$$F>" > /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/tc_ematch/$$F; done; touch /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/tc_ematch/.install