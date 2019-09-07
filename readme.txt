TP-LINK T1600G-28TS(UN) 1.0 GPL code readme

1. This package contains GPL code for T1600G-28TS(UN) 1.0
2. All components have been built successfully on CentOS Linux release 6.0.

Build Instructions
1. All build targets are in "t1600g-28ts_gpl/tplink/buildroot/" and "t1600g-28ts_gpl/ldk/3.4.5_RC3/bootloader/uboot-2012.10", you should enter the directory to build components.

2. Toolchain binary is avaliable in this package. The directory is "t1600g-28ts_gpl/tplink/buildroot/host/usr/bin/".

3. Building steps:
 1) put t1600g-28ts_gpl in directory /project/trunk
 2) export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/project/trunk/t1600g-28ts_gpl/tplink/buildroot/host/usr/lib" 
 3) cd /project/trunk_t1600g-28ts_gpl/tplink/buildroot
 3) make O=./build/t1600g-28ts tplink-t1600g-28ts_defconfig
 4) make O=./build/t1600g-28ts
 After step4 completed, The linux kernel image, rootfs filesystem will be found in directory "t1600g-28ts_gpl/tplink/buildroot/build/t1600g-28ts/image".
 
 5) cd /project/trunk/t1600g-28ts_gpl/ldk/3.4.5_RC3/bootloader/uboot-2012.10
 6) export CROSS_COMPILE=/project/trunk/t1600g-28ts_gpl/tplink/buildroot/host/usr/bin/arm-linux-
 7) export ARCH=arm
 8) export PATH="$PATH:/project/trunk/t1600g-28ts_gpl/tplink/buildroot/host/usr/bin"
 9) make O=./build-output hurricane2_config
 10) make O=./build-output all
 After step11 completed, uboot will be found in directory "t1600g-28ts_gpl/ldk/3.4.5_RC3/bootloader/uboot-2012.10".



