#!/bin/bash

if [ "$#" -lt 2 ]
then
  echo "usage: sudo $0 mon_name osd1_name:data_dir:journal_dev [osd2_name...]"
  echo "example: sudo $0 monitor OSD1:/mnt/osd1:/dev/sdb1 OSD2:/mnt/osd2:/dev/sdb1"
  echo "example: sudo $0 node185 node102:/mnt/osd1:/dev/sdb1 node109:/mnt/osd2:/dev/sdb1"
  exit 0
fi

for osd in $*
do
  if [ "$osd" == "$1" ]
  then
    continue
  fi
  ceph-deploy purge ${osd%%":"*}
  src_dir="/home/jinc/ceph"
  ssh root@${osd%%":"*} "cd $src_dir;make uninstall"
  ceph-deploy purgedata ${osd%%":"*}
  data_dir=`echo $osd|cut -d":" -f2`
  ssh root@${osd%%":"*} "cd $data_dir;rm -fr *"
done

ceph-deploy purge $1
cd /home/jinc/ceph
sudo make uninstall
ceph-deploy purgedata $1

# delete all files except sh scripts in current folder
##ls|grep -v '.sh'|xargs rm -fr
rm -f *.keyring
rm -f ceph.log
rm -f ceph.conf
