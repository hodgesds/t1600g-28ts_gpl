cmd_/Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm-generic/.install := perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/asm-generic /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm-generic arm auxvec.h bitsperlong.h errno-base.h errno.h fcntl.h int-l64.h int-ll64.h ioctl.h ioctls.h ipcbuf.h kvm_para.h mman-common.h mman.h msgbuf.h param.h poll.h posix_types.h resource.h sembuf.h setup.h shmbuf.h shmparam.h siginfo.h signal-defs.h signal.h socket.h sockios.h stat.h statfs.h swab.h termbits.h termios.h types.h ucontext.h unistd.h; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/asm-generic /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm-generic arm ; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/generated/asm-generic /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm-generic arm ; for F in ; do echo "\#include <asm-generic/$$F>" > /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm-generic/$$F; done; touch /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/asm-generic/.install
