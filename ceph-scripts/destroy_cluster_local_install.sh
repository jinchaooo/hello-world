#!/bin/bash
# Author: Jin_Chao@dsi.a-star.edu.sg
# This scripts does not uninstall ceph in each ceph node
# You may mannaully uninstall ceph in each ceph node if you want
# This scripts stops the ceph daemon in each ceph node

if [ "$#" -lt 2 ]
then
  echo "usage: $0 mon_name osd1_name:data_dir:journal_dev [osd2_name...]"
  echo "example: $0 monitor OSD1:/mnt/osd1:/dev/sdb1 OSD2:/mnt/osd2:/dev/sdb1"
  echo "example: $0 node185 node102:/mnt/osd1:/dev/sdb1 node109:/mnt/osd2:/dev/sdb1"
  exit 0
fi

for osd in $*
do
  if [ "$osd" == "$1" ]
  then
    continue
  fi
  # stop the ceph-osd daemon
  osd_ps=`ssh ${osd%%":"*} "ps aux|grep ceph-osd"`
  osd_id=`echo $osd_ps | awk '{for(i=1;i<=NF;i++) if($i=="-i") {i=i+1;print $i;break;}}'`
  ssh ${osd%%":"*} "sudo stop ceph-osd id=$osd_id"
  # purge osd
  ceph-deploy purge ${osd%%":"*}
  ##ceph-deploy purgedata ${osd%%":"*}
  ssh ${osd%%":"*} "sudo rm -fr /var/lib/ceph/*;sudo rm -fr /var/run/ceph;sudo rm -fr /etc/ceph/*"
  # clear the osd data dir
  data_dir=`echo $osd|cut -d":" -f2`
  ssh ${osd%%":"*} "cd $data_dir;sudo rm -fr *"
done

# stop the ceph-mon daemon
mon_ps=`ssh $1 "ps aux|grep ceph-mon"`
mon_id=`echo $mon_ps | awk '{for(i=1;i<=NF;i++) if($i=="-i") {i=i+1;print $i;break;}}'`
ssh $1 "sudo stop ceph-mon id=$mon_id"
# purge mon
ceph-deploy purge $1
##ceph-deploy purgedata $1
ssh $1 "sudo rm -fr /var/lib/ceph/*;sudo rm -fr /var/run/ceph;sudo rm -fr /etc/ceph/*"

# delete all files except sh scripts in current folder
##ls|grep -v '.sh'|xargs rm -fr
rm -f *.keyring
rm -f ceph.log
rm -f ceph.conf
