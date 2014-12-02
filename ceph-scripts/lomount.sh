#!/bin/bash

loop0=`sudo losetup -a|grep /dev/loop0|wc -l`
if [ "$loop0" = "1" ]
then
  echo "/dev/loop0 already set up"
else
  sudo losetup /dev/loop0 /mnt/fda
fi
  
loop1=`sudo losetup -a|grep /dev/loop1|wc -l`
if [ "$loop1" = "1" ]
then
  echo "/dev/loop1 already set up"
else
  sudo losetup /dev/loop1 /mnt/fdb
fi

if [ $# -eq 1 -a "$1" = "u" ] 
then
  sudo umount -l /dev/loop0
  sudo umount -l /dev/loop1
  exit 0
fi

if [ $# -eq 1 ]
then
  case "$1" in
    "btrfs")
      MKFS="mkfs.btrfs -f"
      ;;
    "ext4")
      MKFS="mkfs.ext4"
      ;;
    "xfs")
      MKFS="mkfs.xfs -f"
      ;;
  esac 
fi

if mountpoint -q /mnt/os
then 
  echo "/mnt/os already mounted"
else
  sudo $MKFS /dev/loop0
  sudo mount -t $1 /dev/loop0 /mnt/os
fi

if mountpoint -q /mnt/cache
then 
  echo "/mnt/cache already mounted"
else
  sudo $MKFS /dev/loop1
  sudo mount -t $1 /dev/loop1 /mnt/cache
fi
