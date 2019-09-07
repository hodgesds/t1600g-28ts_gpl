#!/bin/bash

# should be run from buildroot directory
# $1 = top_dir $2 = build_dir $3 = bcm_file_name

FILENAME=$3
packagedir=package
outputdir=$2

echo "Patching the bcm files Top = $1 Build = $2 bcmfile = $3"

if [ ! -d $outputdir ]
then
	mkdir -p $outputdir
fi

count=1;

grep -iv "LDKINTERNAL"  $FILENAME | while read line
do
	dirname=`basename "$line"`
	packagedirfile="$packagedir/$dirname"
	outputdirfile="$outputdir/$dirname"

	let "count += 1"

	if [ $count -gt 3 ];
	then
		if [ ! -e $packagedirfile ]
		then
			## fake a package directory
			ln -s $1/../$line $packagedirfile
			
		fi
		## fake a tar file for legal-info
		echo "" > ./dl/$dirname-undefined.tar.gz
		outputdirfile="$outputdir/$dirname-undefined"
	fi



	#echo $line $outputdirfile
	if [ ! -e $outputdirfile ]
	then
		## fake an untarred build directory
		ln -s $1/../$line $outputdirfile

		rm -f $outputdirfile/.stamp_images_*
		rm -f $outputdirfile/.stamp_target_*
		rm -f $outputdirfile/.stamp*

		touch $outputdirfile/.stamp_extracted
		touch $outputdirfile/.stamp_downloaded
		touch $outputdirfile/.stamp_patched
	fi
done
