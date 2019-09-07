#############################################################
#
# app demo
#
#############################################################
APP_DEMO_VERSION = 1.0
APP_DEMO_SITE = $(BR2_TPLINK_SOURCE_DIR_STRIP)/app/demo
APP_DEMO_SITE_METHOD = local
APP_DEMO_AUTORECONF = YES
APP_DEMO_INSTALL_STAGING = NO
APP_DEMO_INSTALL_TARGET = YES
APP_DEMO_CONF_OPT =  --disable-static
APP_DEMO_CFLAGS += -g 

define APP_DEMO_INSTALL_TARGET_CMDS
	 cp -a $(APP_DEMO_SRCDIR)/*.a  $(BR2_TPLINK_SOURCE_DIR_STRIP)/archive/$(BR2_TPLINK_PROJECT_NAME)/
endef

APP_DEMO_LDFLAGS +=-lpthread -lm -lrt 

APP_DEMO_CONF_ENV += CFLAGS="$(APP_DEMO_CFLAGS)" 
APP_DEMO_CONF_ENV += LDFLAGS="$(APP_DEMO_LDFLAGS)"

ifeq ($(BR2_TPLINK_APP_DEMO),y)
CORE_APP_LIBS += app_demo.a
endif

$(eval $(tplink-autotools-package))
