#!/bin/bash
if [ $# -eq 1 -a "$1" = "u" ]
then
  sudo umount -l /mnt/os
  sudo umount -l /mnt/jnr
  sudo umount -l /mnt/cache
  exit 0
fi

osdev="/dev/sdd1"
jnrdev="/dev/sde3"
cachedev="/dev/sde2"

mkdir -p /mnt/os
mkdir -p /mnt/jnr
mkdir -p /mnt/cache

if [ $# -ne 1 ]
then 
  echo "usage: $0 btrfs/ext4/xfs"
  echo "mkfs on $osdev (hdd) => /mnt/os,"
  echo "        $jnrdev (ssd) => /mnt/jnr,"
  echo "        $cachedev (ssd) => /mnt/cache,"
  exit 0
fi

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

if mountpoint -q /mnt/os
then
  echo "/mnt/os already mounted"
else
  echo "mkfs on $osdev mount to /mnt/os"
  sudo $MKFS $osdev
  sudo mount -t "$1" $osdev /mnt/os
fi

if mountpoint -q /mnt/jnr
then
  echo "/mnt/jnr already mounted"
else
  echo "mkfs on $jnrdev mount to /mnt/jnr"
  sudo $MKFS $jnrdev
  sudo mount -t "$1" $jnrdev /mnt/jnr
fi

if mountpoint -q /mnt/cache
then
  echo "/mnt/cache already mounted"
else
  echo "mkfs on $cachedev mount to /mnt/cache"
  sudo $MKFS $cachedev
  sudo mount -t "$1" $cachedev /mnt/cache
fi
