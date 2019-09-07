#!/bin/bash

# ./tplink_patch_clean.sh $(BUILD_DIR)

tplink_build_dir=$1/tplink
echo "clean the tplink patch"

if [ -d $tplink_build_dir ]
then
    rm -rf $tplink_build_dir
fi

    
