#!/bin/bash

if [ "$#" -eq 1 -a "$1" == "u" ]
then
  echo "umount all folders"
  sudo umount /data/gfs1
  sudo umount /data/gfs2
  sudo umount /data/gfs3
  sudo umount /data/gfs4
  sudo umount /data/gfs5
  sudo umount /data/gfs6
  sudo umount /data/gfs7
  sudo umount /data/gfs8
  sudo umount /data/ssd
  exit
fi

if [ "$#" -eq 1 -a "$1" == "m" ]
then
  echo "mount all folders"
  #200G
  sudo mount /dev/sdb1 /data/gfs1
  sudo mount /dev/sdb2 /data/gfs2
  sudo mount /dev/sdb3 /data/gfs3
  sudo mount /dev/sdb4 /data/gfs4
  sudo mount /dev/sdc1 /data/gfs5
  sudo mount /dev/sdc2 /data/gfs6
  sudo mount /dev/sdc3 /data/gfs7
  sudo mount /dev/sdc4 /data/gfs8
  #20G
  sudo mount /dev/sde1 /data/ssd
  exit
fi

if [ "$#" -eq 1 -a "$1" == "c" ]
then
  echo "clear all folders"
  #200G
  sudo rm -fr /data/gfs1/*
  sudo rm -fr /data/gfs2/*
  sudo rm -fr /data/gfs3/*
  sudo rm -fr /data/gfs4/*
  sudo rm -fr /data/gfs5/*
  sudo rm -fr /data/gfs6/*
  sudo rm -fr /data/gfs7/*
  sudo rm -fr /data/gfs8/*
  #20G
  sudo rm -fr /data/ssd/*
  exit
fi

echo "usage: $0 m | u | c"
