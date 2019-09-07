#############################################################
#
# cpio to archive target filesystem
#
#############################################################

ifeq ($(BR2_ROOTFS_DEVICE_CREATION_STATIC),y)

define ROOTFS_CPIO_ADD_INIT
        if [ ! -e $(TARGET_DIR)/init ]; then \
                ln -sf sbin/init $(TARGET_DIR)/init; \
        fi
endef

else
# devtmpfs does not get automounted when initramfs is used.
# Add a pre-init script to mount it before running init
define ROOTFS_CPIO_ADD_INIT
        if [ ! -e $(TARGET_DIR)/init ]; then \
                $(INSTALL) -m 0755 fs/cpio/init $(TARGET_DIR)/init; \
        fi
endef

PACKAGES_PERMISSIONS_TABLE += /dev/console c 622 0 0 5 1 - - -$(sep)

endif # BR2_ROOTFS_DEVICE_CREATION_STATIC

ROOTFS_CPIO_PRE_GEN_HOOKS += ROOTFS_CPIO_ADD_INIT

define ROOTFS_CPIO_CMD
	cp -ar $(BR2_TPLINK_SOURCE_DIR_STRIP)/archive/$(BR2_TPLINK_PROJECT_NAME)/usr/lib/*  $(TARGET_DIR)/tplink/lib/ &&\
	rm -rf $(TARGET_DIR)/tplink/lib/*.so $(TARGET_DIR)/tplink/lib/*.la $(TARGET_DIR)/tplink/lib/*.a &&\
	rename .0.0.0 .0 $(TARGET_DIR)/tplink/lib/*.0.0.0 &&\
	rename .0.0 .0 $(TARGET_DIR)/tplink/lib/*.0.0 &&\
	rm -rf $(TARGET_DIR)/usr/arm-buildroot-linux-uclibcgnueabi &&\
	rm -rf $(TARGET_DIR)/usr/bin/addr2line &&\
	rm -rf $(TARGET_DIR)/usr/bin/ar &&\
	rm -rf $(TARGET_DIR)/usr/bin/arping &&\
	rm -rf $(TARGET_DIR)/usr/bin/as &&\
	rm -rf $(TARGET_DIR)/usr/bin/basename &&\
	rm -rf $(TARGET_DIR)/usr/bin/bzcat &&\
	rm -rf $(TARGET_DIR)/usr/bin/c++filt &&\
	rm -rf $(TARGET_DIR)/usr/bin/cal &&\
	rm -rf $(TARGET_DIR)/usr/bin/chkdupexe &&\
	rm -rf $(TARGET_DIR)/usr/bin/chrt &&\
	rm -rf $(TARGET_DIR)/usr/bin/chvt &&\
	rm -rf $(TARGET_DIR)/usr/bin/cksum &&\
	rm -rf $(TARGET_DIR)/usr/bin/clear &&\
	rm -rf $(TARGET_DIR)/usr/bin/cmp &&\
	rm -rf $(TARGET_DIR)/usr/bin/col &&\
	rm -rf $(TARGET_DIR)/usr/bin/colcrt &&\
	rm -rf $(TARGET_DIR)/usr/bin/colrm &&\
	rm -rf $(TARGET_DIR)/usr/bin/column &&\
	rm -rf $(TARGET_DIR)/usr/bin/crontab &&\
	rm -rf $(TARGET_DIR)/usr/bin/cut &&\
	rm -rf $(TARGET_DIR)/usr/bin/cytune &&\
	rm -rf $(TARGET_DIR)/usr/bin/dc &&\
	rm -rf $(TARGET_DIR)/usr/bin/deallocvt &&\
	rm -rf $(TARGET_DIR)/usr/bin/diff &&\
	rm -rf $(TARGET_DIR)/usr/bin/dirname &&\
	rm -rf $(TARGET_DIR)/usr/bin/eject &&\
	rm -rf $(TARGET_DIR)/usr/bin/elfedit &&\
	rm -rf $(TARGET_DIR)/usr/bin/env &&\
	rm -rf $(TARGET_DIR)/usr/bin/ether-wake &&\
	rm -rf $(TARGET_DIR)/usr/bin/expr &&\
	rm -rf $(TARGET_DIR)/usr/bin/fdformat &&\
	rm -rf $(TARGET_DIR)/usr/bin/find &&\
	rm -rf $(TARGET_DIR)/usr/bin/flock &&\
	rm -rf $(TARGET_DIR)/usr/bin/fold &&\
	rm -rf $(TARGET_DIR)/usr/bin/fuser &&\
	rm -rf $(TARGET_DIR)/usr/bin/getopt &&\
	rm -rf $(TARGET_DIR)/usr/bin/gprof &&\
	rm -rf $(TARGET_DIR)/usr/bin/head &&\
	rm -rf $(TARGET_DIR)/usr/bin/hexdump &&\
	rm -rf $(TARGET_DIR)/usr/bin/hostid &&\
	rm -rf $(TARGET_DIR)/usr/bin/id &&\
	rm -rf $(TARGET_DIR)/usr/bin/install &&\
	rm -rf $(TARGET_DIR)/usr/bin/ipcmk &&\
	rm -rf $(TARGET_DIR)/usr/bin/ipcrm &&\
	rm -rf $(TARGET_DIR)/usr/bin/ipcs &&\
	rm -rf $(TARGET_DIR)/usr/bin/isosize &&\
	rm -rf $(TARGET_DIR)/usr/bin/killall &&\
	rm -rf $(TARGET_DIR)/usr/bin/killall5 &&\
	rm -rf $(TARGET_DIR)/usr/bin/last &&\
	rm -rf $(TARGET_DIR)/usr/bin/ld &&\
	rm -rf $(TARGET_DIR)/usr/bin/ld.bfd &&\
	rm -rf $(TARGET_DIR)/usr/bin/less &&\
	rm -rf $(TARGET_DIR)/usr/bin/linux32 &&\
	rm -rf $(TARGET_DIR)/usr/bin/linux64 &&\
	rm -rf $(TARGET_DIR)/usr/bin/logger &&\
	rm -rf $(TARGET_DIR)/usr/bin/logname &&\
	rm -rf $(TARGET_DIR)/usr/bin/look &&\
	rm -rf $(TARGET_DIR)/usr/bin/lscpu &&\
	rm -rf $(TARGET_DIR)/usr/bin/lspci &&\
	rm -rf $(TARGET_DIR)/usr/bin/lsusb &&\
	rm -rf $(TARGET_DIR)/usr/bin/lzcat &&\
	rm -rf $(TARGET_DIR)/usr/bin/make &&\
	rm -rf $(TARGET_DIR)/usr/bin/mcookie &&\
	rm -rf $(TARGET_DIR)/usr/bin/md5sum &&\
	rm -rf $(TARGET_DIR)/usr/bin/mesg &&\
	rm -rf $(TARGET_DIR)/usr/bin/microcom &&\
	rm -rf $(TARGET_DIR)/usr/bin/mkfifo &&\
	rm -rf $(TARGET_DIR)/usr/bin/namei &&\
	rm -rf $(TARGET_DIR)/usr/bin/nm &&\
	rm -rf $(TARGET_DIR)/usr/bin/nohup &&\
	rm -rf $(TARGET_DIR)/usr/bin/nslookup &&\
	rm -rf $(TARGET_DIR)/usr/bin/objcopy &&\
	rm -rf $(TARGET_DIR)/usr/bin/objdump &&\
	rm -rf $(TARGET_DIR)/usr/bin/od &&\
	rm -rf $(TARGET_DIR)/usr/bin/openvt &&\
	rm -rf $(TARGET_DIR)/usr/bin/patch &&\
	rm -rf $(TARGET_DIR)/usr/bin/printf &&\
	rm -rf $(TARGET_DIR)/usr/bin/ranlib &&\
	rm -rf $(TARGET_DIR)/usr/bin/readelf &&\
	rm -rf $(TARGET_DIR)/usr/bin/realpath &&\
	rm -rf $(TARGET_DIR)/usr/bin/renice &&\
	rm -rf $(TARGET_DIR)/usr/bin/resize &&\
	rm -rf $(TARGET_DIR)/usr/bin/rev &&\
	rm -rf $(TARGET_DIR)/usr/bin/script &&\
	rm -rf $(TARGET_DIR)/usr/bin/scriptreplay &&\
	rm -rf $(TARGET_DIR)/usr/bin/setarch &&\
	rm -rf $(TARGET_DIR)/usr/bin/setkeycodes &&\
	rm -rf $(TARGET_DIR)/usr/bin/setsid &&\
	rm -rf $(TARGET_DIR)/usr/bin/sha1sum &&\
	rm -rf $(TARGET_DIR)/usr/bin/sha256sum &&\
	rm -rf $(TARGET_DIR)/usr/bin/sha512sum &&\
	rm -rf $(TARGET_DIR)/usr/bin/size &&\
	rm -rf $(TARGET_DIR)/usr/bin/sort &&\
	rm -rf $(TARGET_DIR)/usr/bin/strace &&\
	rm -rf $(TARGET_DIR)/usr/bin/strace-log-mer &&\
	rm -rf $(TARGET_DIR)/usr/bin/strings &&\
	rm -rf $(TARGET_DIR)/usr/bin/strip &&\
	rm -rf $(TARGET_DIR)/usr/bin/tail &&\
	rm -rf $(TARGET_DIR)/usr/bin/tailf &&\
	rm -rf $(TARGET_DIR)/usr/bin/tar &&\
	rm -rf $(TARGET_DIR)/usr/bin/tee &&\
	rm -rf $(TARGET_DIR)/usr/bin/telnet &&\
	rm -rf $(TARGET_DIR)/usr/bin/test &&\
	rm -rf $(TARGET_DIR)/usr/bin/tftp &&\
	rm -rf $(TARGET_DIR)/usr/bin/time &&\
	rm -rf $(TARGET_DIR)/usr/bin/top &&\
	rm -rf $(TARGET_DIR)/usr/bin/tr &&\
	rm -rf $(TARGET_DIR)/usr/bin/traceroute &&\
	rm -rf $(TARGET_DIR)/usr/bin/traceroute6 &&\
	rm -rf $(TARGET_DIR)/usr/bin/tty &&\
	rm -rf $(TARGET_DIR)/usr/bin/uniq &&\
	rm -rf $(TARGET_DIR)/usr/bin/unix2dos &&\
	rm -rf $(TARGET_DIR)/usr/bin/unlzma &&\
	rm -rf $(TARGET_DIR)/usr/bin/unxz &&\
	rm -rf $(TARGET_DIR)/usr/bin/uptime &&\
	rm -rf $(TARGET_DIR)/usr/bin/uudecode &&\
	rm -rf $(TARGET_DIR)/usr/bin/uuencode &&\
	rm -rf $(TARGET_DIR)/usr/bin/uuidgen &&\
	rm -rf $(TARGET_DIR)/usr/bin/vlock &&\
	rm -rf $(TARGET_DIR)/usr/bin/wget &&\
	rm -rf $(TARGET_DIR)/usr/bin/whereis &&\
	rm -rf $(TARGET_DIR)/usr/bin/which &&\
	rm -rf $(TARGET_DIR)/usr/bin/who &&\
	rm -rf $(TARGET_DIR)/usr/bin/whoami &&\
	rm -rf $(TARGET_DIR)/usr/bin/xargs &&\
	rm -rf $(TARGET_DIR)/usr/bin/xz &&\
	rm -rf $(TARGET_DIR)/usr/bin/xzcat &&\
	rm -rf $(TARGET_DIR)/usr/bin/yes &&\
	rm -rf $(TARGET_DIR)/usr/sbin/brctl &&\
	rm -rf $(TARGET_DIR)/usr/sbin/chroot &&\
	rm -rf $(TARGET_DIR)/usr/sbin/crond &&\
	rm -rf $(TARGET_DIR)/usr/sbin/dnsd &&\
	rm -rf $(TARGET_DIR)/usr/sbin/fdformat &&\
	rm -rf $(TARGET_DIR)/usr/sbin/flashcp &&\
	rm -rf $(TARGET_DIR)/usr/sbin/flash_lock &&\
	rm -rf $(TARGET_DIR)/usr/sbin/flash_unlock &&\
	rm -rf $(TARGET_DIR)/usr/sbin/klogd &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ldattach &&\
	rm -rf $(TARGET_DIR)/usr/sbin/loadfont &&\
	rm -rf $(TARGET_DIR)/usr/sbin/nanddump &&\
	rm -rf $(TARGET_DIR)/usr/sbin/nandtest &&\
	rm -rf $(TARGET_DIR)/usr/sbin/nandwrite &&\
	rm -rf $(TARGET_DIR)/usr/sbin/rdate &&\
	rm -rf $(TARGET_DIR)/usr/sbin/readprofile &&\
	rm -rf $(TARGET_DIR)/usr/sbin/rtcwake &&\
	rm -rf $(TARGET_DIR)/usr/sbin/setlogcons &&\
	rm -rf $(TARGET_DIR)/usr/sbin/syslogd &&\
	rm -rf $(TARGET_DIR)/usr/sbin/tunelp &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubiattach &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubicrc32 &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubidetach &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubiformat &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubimkvol &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubinfo &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubinize &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubirename &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubirmvol &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubirsvol &&\
	rm -rf $(TARGET_DIR)/usr/sbin/ubiupdatevol &&\
	rm -rf $(TARGET_DIR)/usr/lib/ldscripts &&\
	rm -rf $(TARGET_DIR)/usr/lib/libelf.so &&\
	rm -rf $(TARGET_DIR)/usr/lib/libip4tc.so &&\
	rm -rf $(TARGET_DIR)/usr/lib/libip6tc.so &&\
	rm -rf $(TARGET_DIR)/usr/lib/libiptc.so &&\
	rm -rf $(TARGET_DIR)/usr/lib/libkmod.so &&\
	rm -rf $(TARGET_DIR)/usr/lib/liblzo2.so &&\
	rm -rf $(TARGET_DIR)/usr/lib/libstdc++.so.6 &&\
	rm -rf $(TARGET_DIR)/usr/lib/libxtables.so &&\
	rm -rf $(TARGET_DIR)/usr/lib/libz.so &&\
	rm -rf $(TARGET_DIR)/../usrfs &&\
	mkdir -p $(TARGET_DIR)/../usrfs &&\
	mkdir -p $(TARGET_DIR)/../usrfs/image &&\
	mkdir -p $(TARGET_DIR)/../usrfs/app &&\
	mkdir -p $(TARGET_DIR)/../usrfs/exe &&\
	mkdir -p $(TARGET_DIR)/../usrfs/image/kernel &&\
	mkdir -p $(TARGET_DIR)/../usrfs/image/usrImage &&\
	mkdir -p $(TARGET_DIR)/../usrfs/image/usrapp &&\
	mkdir -p $(TARGET_DIR)/../usrfs/app/conf &&\
	mkdir -p $(TARGET_DIR)/../usrfs/app/log &&\
	mkdir -p $(TARGET_DIR)/../usrfs/app/data &&\
	cp -rf $(TARGET_DIR)/tplink/*  $(TARGET_DIR)/../usrfs/exe &&\
	cp -rf $(TARGET_DIR)/../usrfs/exe/data/ca.* $(TARGET_DIR)/../usrfs/app/data &&\
	cd $(TARGET_DIR)/../usrfs/exe/ && \
	cp -rf $(TARGET_DIR)/../usrfs/exe/* $(TARGET_DIR)/../usrfs/image/usrImage/&&\
	rm -rf $(TARGET_DIR)/tplink/* &&\
	cd $(TARGET_DIR) && find . | cpio --quiet -o -H newc | lzma > $$@
endef

$(eval $(call ROOTFS_TARGET,cpio))
