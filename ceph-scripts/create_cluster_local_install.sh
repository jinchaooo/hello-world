#!/bin/bash
# Author: Jin_Chao@dsi.a-star.edu.sg
# First you need to install ceph mannually from src in all ceph nodes
# Do not run this script as root or sudo
# You should create the same USER in all ceph nodes
# In each ceph nodes, the USER should be in password-less soduer list
# This script calls ceph-deploy, 
# which will ssh into each ceph nodes as USER, and use sudo when necessary

if [ "$#" -lt 2 ]
then
  echo "usage: $0 mon_name osd1_name:data_dir:journal_dev [osd2_name...]"
  echo "example: $0 monitor OSD1:/mnt/osd1:/dev/sdb1 OSD2:/mnt/osd2:/dev/sdb1"
  echo "example: $0 node185 node102:/mnt/osd1:/dev/sdb1 node109:/mnt/osd2:/dev/sdb1"
  exit 0
fi

ceph-deploy new $1
sed -i 's/cephx/none/g' ceph.conf
sed -i '$ i\osd pool default size = 1' ceph.conf
##sed -i '$ i\filestore max inline xattr size = 0' ceph.conf
##sed -i '$ i\filestore max inline xattrs = 0' ceph.conf
# perform a local 'make install' instead
##ceph-deploy install $1
ceph-deploy mon create-initial

for osd in $*
do
  if [ "$osd" == "$1" ]
  then
    continue
  fi
  # perform a local 'make install' instead
  ##ceph-deploy install ${osd%%":"*}
  # create the dirs
  dir="/var/lib/ceph/bootstrap-osd"
  ssh ${osd%%":"*} "if [ ! -d $dir ];then sudo mkdir -p $dir;fi"
  dir="/var/lib/ceph/osd"
  ssh ${osd%%":"*} "if [ ! -d $dir ];then sudo mkdir -p $dir;fi"
  dir="/etc/ceph"
  ssh ${osd%%":"*} "if [ ! -d $dir ];then sudo mkdir -p $dir;fi"
  ceph-deploy osd prepare $osd
  ceph-deploy osd activate $osd
done
