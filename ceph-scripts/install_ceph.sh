#!/bin/bash

if [ "$#" -lt 2 -o "$2" != "y" ]
then
  echo "make sure to run this script in top src folder of ceph, if ok, run with: $0 y"
  exit 0
fi

./autogen.sh

./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var

make -j `grep -c ^processor /proc/cpuinfo` 

sudo make install

sudo mkdir -p /var/lib/ceph/osd

sudo mkdir -p /var/lib/ceph/bootstrap-osd

sudo cp src/upstart/ceph*.conf /etc/init/
