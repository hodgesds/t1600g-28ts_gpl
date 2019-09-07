#!/bin/sh


CheckProcess()  
{  
    if [ "$1" = "" ];  
    then  
	    return 1  
    fi  
  
    PROCESS_NUM=`ps -ef | grep "$1" | grep -v "grep" | wc -l`  
    if [ $PROCESS_NUM -eq 1 ];  
    then  
        return 0  
    else  
        return 1  
    fi      
}  

InitCore()
{
	AREA=`cat /proc/mtd | grep "usrimg2@main" | awk '{print $1}'`
	if [ "$AREA" = "mtd4:" ];
        then
          mount -t jffs2 /dev/mtdblock4 /tplink
        else
          mount -t jffs2 /dev/mtdblock3 /tplink
        fi
        
        CHKFLAG=0       
        c1="/tplink/usrImage/app/sbin/core"
        c2="/tplink/usrImage/app/sbin/cli_server"
        c3="/tplink/usrImage/app/sbin/monitor.cfg"
        c4="/tplink/usrImage/app/sbin/scm"
        c5="/lib/modules/linux-kernel-bde.ko"
        c6="/lib/modules/linux-user-bde.ko"
        c7="/lib/modules/ethdriver.ko"
	if [ ! -f "$c1" ];then
          CHKFLAG=1		
	fi
	if [ ! -f "$c1" ];then
          CHKFLAG=1		
	fi
	if [ ! -f "$c2" ];then
          CHKFLAG=1		
	fi
	if [ ! -f "$c3" ];then
          CHKFLAG=1		
	fi
	if [ ! -f "$c4" ];then
          CHKFLAG=1		
	fi
	if [ ! -f "$c5" ];then
          CHKFLAG=1		
	fi
	if [ ! -f "$c6" ];then
          CHKFLAG=1		
	fi
	if [ ! -f "$c7" ];then
          CHKFLAG=1		
	fi
	if [ $CHKFLAG -eq 1 ];
	then
	    echo "check area fail ,mount anther area!"
            if [ "$AREA" = "mtd4:" ];
            then
              mount -t jffs2 /dev/mtdblock3 /tplink
            else
              mount -t jffs2 /dev/mtdblock4 /tplink
            fi
        fi	

	mount -t jffs2 /dev/mtdblock5 /tplink/usrapp
	
	exepath="/tplink/exe"	
	if [ ! -d "$exepath" ];then
	   mkdir $exepath
	fi
	

	
	ln -sf /tplink/usrImage/* /tplink/exe		
		
	mknod /dev/linux-bcm-diag c 124 0
	ln -sf /dev/linux-bcm-diag /dev/linux-bcm-diag-full
	mknod /dev/linux-uk-proxy c 125 0
	mknod /dev/linux-user-bde c 126 0
	mknod /dev/linux-kernel-bde c 127 0
	mknod /dev/tp-eth c 127 0
		
	export LD_LIBRARY_PATH=/tplink/exe/lib
	export PATH="$PATH:/tplink/exe/app/sbin"
}
   
while [ 1 ] ; do  
    InitCore	
    CheckProcess "monitor"  
    CheckQQ_RET=$?  
    if [ $CheckQQ_RET -eq 1 ];  
    then  
     killall -9 monitor 1> /dev/nul 2>&1
     cd /etc
     ./monitor  
    fi  
    sleep 2  
done  



