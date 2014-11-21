#!/bin/bash

if [ "$#" -lt 1 ]
then
  echo "make sure to run this script in top src folder of ceph"
  echo "usage: $0 options"
  echo "c    do configure only"
  echo "m    do make only"
  echo "i    do install only"
  echo "u    do uninstall only"
  echo "cm   do configure and make"
  echo "mi   do make and install"
  echo "cmi  do configure, make and install"
  exit 0
fi

if [ "$1" == "c" -o "$1" == "cm" -o "$1" == "cmi" ]
then
./autogen.sh
./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var
fi

if [ "$1" == "m" -o "$1" == "cm" -o "$1" == "mi" -o "$1" == "cmi" ]
then
make -j `grep -c ^processor /proc/cpuinfo` 
fi

if [ "$1" == "i" -o "$1" == "mi" -o "$1" == "cmi" ]
then
sudo make install
sudo mkdir -p /var/lib/ceph/osd
sudo mkdir -p /var/lib/ceph/bootstrap-osd
sudo cp src/upstart/ceph*.conf /etc/init/
fi

if [ "$1" == "u" ]
then
sudo make uninstall
fi

