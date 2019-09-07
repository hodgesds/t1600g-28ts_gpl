#!/bin/bash
# $1 = top_dir $2 = build_dir $3 = bcm_file_name
./bcm_patch.sh $1 $2 $3 

## some hack for clean
make -C $2/linux-3.6.5 ARCH=arm clean
rm -f $2/linux-3.6.5/arch/arm/configs/buildroot_defconfig
#make -C output/build/uboot-2012.10  distclean
#rm   -f output/build/uboot-2012.10/*_config
