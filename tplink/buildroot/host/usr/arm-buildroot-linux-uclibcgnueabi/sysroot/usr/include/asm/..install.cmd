cmd_/Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm/.install := perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/arch/arm/include/asm /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm arm a.out.h byteorder.h fcntl.h hwcap.h ioctls.h ipcbuf.h kvm_para.h mman.h msgbuf.h param.h posix_types.h ptrace.h sembuf.h setup.h shmbuf.h sigcontext.h signal.h socket.h sockios.h stat.h statfs.h swab.h termbits.h termios.h types.h unistd.h; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/arch/arm/include/asm /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm arm ; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/arch/arm/include/generated/asm /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm arm ; for F in auxvec.h bitsperlong.h errno.h ioctl.h poll.h resource.h siginfo.h; do echo "\#include <asm-generic/$$F>" > /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm/$$F; done; touch /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm/.install