cmd_/Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/scsi/.install := perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/scsi /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/scsi arm scsi_bsg_fc.h scsi_netlink.h scsi_netlink_fc.h; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/scsi /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/scsi arm ; perl scripts/headers_install.pl /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux-3.6.5/include/generated/scsi /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/scsi arm ; for F in ; do echo "\#include <asm-generic/$$F>" > /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/scsi/$$F; done; touch /Project/Trunk/Src_Linux/tplink/buildroot/output/toolchain/linux/include/scsi/.install
