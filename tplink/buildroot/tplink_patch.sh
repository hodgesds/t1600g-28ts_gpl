#!/bin/bash

# ./tplink_patch.sh $(TOPDIR) $(BUILD_DIR) $(BR2_TPLINK_PROJECT_NAME)

tplink_build_dir=$2/tplink
tplink_archive_dir=$2/tplink/archive
tplink_include_dir=$2/tplink/include
archivedir=$1/../src/archive
includedir=$1/../src/include

if [ ! -d $archivedir/$3 ]
then
	mkdir -p $archivedir/$3
fi
if [ ! -d $tplink_build_dir ]
then
	mkdir -p $tplink_build_dir
    ln -s $archivedir/$3 $tplink_archive_dir
    ln -s $includedir $tplink_include_dir
fi

   
