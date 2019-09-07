#############################################################
#
# Build the jffs2 root filesystem image
#
#############################################################

JFFS2_OPTS := -e $(BR2_TARGET_ROOTFS_JFFS2_EBSIZE)
SUMTOOL_OPTS := $(JFFS2_OPTS)

ifeq ($(BR2_TARGET_ROOTFS_JFFS2_PAD),y)
ifneq ($(strip $(BR2_TARGET_ROOTFS_JFFS2_PADSIZE)),0x0)
JFFS2_OPTS += --pad=$(strip $(BR2_TARGET_ROOTFS_JFFS2_PADSIZE))
else
JFFS2_OPTS += -p
endif
SUMTOOL_OPTS += -p
endif

ifeq ($(BR2_TARGET_ROOTFS_JFFS2_LE),y)
JFFS2_OPTS += -l
SUMTOOL_OPTS += -l
endif

ifeq ($(BR2_TARGET_ROOTFS_JFFS2_BE),y)
JFFS2_OPTS += -b
SUMTOOL_OPTS += -b
endif

JFFS2_OPTS += -s $(BR2_TARGET_ROOTFS_JFFS2_PAGESIZE)
ifeq ($(BR2_TARGET_ROOTFS_JFFS2_NOCLEANMARKER),y)
JFFS2_OPTS += -n
SUMTOOL_OPTS += -n
endif

ROOTFS_JFFS2_DEPENDENCIES = host-mtd

ifneq ($(BR2_TARGET_ROOTFS_JFFS2_SUMMARY),)
define ROOTFS_JFFS2_CMD
	$(MKFS_JFFS2) $(JFFS2_OPTS) -d $(TARGET_DIR) -o $$@.nosummary && \
	$(SUMTOOL) $(SUMTOOL_OPTS) -i $$@.nosummary -o $$@ && \
	cp -rf $(TARGET_DIR)/../usrfs/exe/* $(TARGET_DIR)/tplink/ && \
	rm $$@.nosummary
endef
else
define ROOTFS_JFFS2_CMD
	$(MKFS_JFFS2) $(JFFS2_OPTS) -d $(TARGET_DIR) -o $$@ && \
	cp -rf $(TARGET_DIR)/../usrfs/exe/* $(TARGET_DIR)/tplink/ && \
	cd $(TARGET_DIR)/../usrfs/exe/ && \
	zip -r $(TARGET_DIR)/../usrfs/image/usrImage/usrImage.bin ./* &&\
	rm -rf $(TARGET_DIR)/../usrfs/exe
endef
endif

ifeq ($(BR2_TPLINK_BUILD_SYSTEM),y)
define ROOTFS_JFFS2_TPLINK_PATCH
	cp -ar $(BR2_TPLINK_SOURCE_DIR_STRIP)/archive/$(BR2_TPLINK_PROJECT_NAME)/usr/lib/*  $(TARGET_DIR)/tplink/lib/
	rm -rf $(TARGET_DIR)/../usrfs
	mkdir -p $(TARGET_DIR)/../usrfs
	mkdir -p $(TARGET_DIR)/../usrfs/image
	mkdir -p $(TARGET_DIR)/../usrfs/app
	mkdir -p $(TARGET_DIR)/../usrfs/exe
	mkdir -p $(TARGET_DIR)/../usrfs/image/kernel
	mkdir -p $(TARGET_DIR)/../usrfs/image/usrImage
	mkdir -p $(TARGET_DIR)/../usrfs/image/usr
	mkdir -p $(TARGET_DIR)/../usrfs/app/conf
	mkdir -p $(TARGET_DIR)/../usrfs/app/log
	mkdir -p $(TARGET_DIR)/../usrfs/app/data       
	cp -rf $(TARGET_DIR)/tplink/*  $(TARGET_DIR)/../usrfs/exe
	rm -rf $(TARGET_DIR)/tplink/*
endef
ROOTFS_JFFS2_PRE_GEN_HOOKS += ROOTFS_JFFS2_TPLINK_PATCH 
endif
$(eval $(call ROOTFS_TARGET,jffs2))
