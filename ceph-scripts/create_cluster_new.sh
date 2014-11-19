#!/bin/bash

if [ "$#" -lt 2 ]
then
  echo "usage: sudo $0 mon_name osd1_name:data_dir:journal_dev [osd2_name...]"
  echo "example: sudo $0 monitor OSD1:/mnt/osd1:/dev/sdb1 OSD2:/mnt/osd2:/dev/sdb1"
  exit 0
fi

ceph-deploy new $1
sed -i 's/cephx/none/g' ceph.conf
sed -i '$ i\osd pool default size = 1' ceph.conf
sed -i '$ i\filestore max inline xattr size = 0' ceph.conf
sed -i '$ i\filestore max inline xattrs = 0' ceph.conf
ceph-deploy install $1
# set unlimited respawn
sed -i '/^respawn limit/d' /etc/init/ceph-osd.conf
###ceph-deploy --overwrite-conf config push $1
ceph-deploy mon create-initial

for osd in $*
do
  if [ "$osd" == "$1" ]
  then
    continue
  fi
  ceph-deploy install ${osd%%":"*}
  scp /etc/init/ceph-osd.conf root@${osd%%":"*}:/etc/init/
  ceph-deploy osd prepare $osd
  ceph-deploy osd activate $osd
done




