cmd_/Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/sunrpc/.install := perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/linux/sunrpc /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/sunrpc arm debug.h; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/linux/sunrpc /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/sunrpc arm ; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/generated/linux/sunrpc /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/sunrpc arm ; for F in ; do echo "\#include <asm-generic/$$F>" > /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/sunrpc/$$F; done; touch /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/linux/sunrpc/.install