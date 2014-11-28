#!/bin/bash
if [ $# -eq 1 -a "$1" = "u" ]
then
  sudo umount -l /mnt/os
  sudo umount -l /mnt/jnr
  sudo umount -l /mnt/cache
  exit 0
fi

if [ $# -ne 1 ]
then 
  echo "usage: $0 btrfs/ext4/xfs"
  echo "mkfs on /dev/sdd1 (hdd) => /mnt/os,"
  echo "        /dev/sde3 (ssd) => /mnt/jnr,"
  echo "        /dev/sde2 (ssd) => /mnt/cache,"
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
  echo "mkfs on /dev/sdd1 mount to /mnt/os"
  sudo $MKFS /dev/sdd1
  sudo mount -t "$1" /dev/sdd1 /mnt/os
fi

if mountpoint -q /mnt/jnr
then
  echo "/mnt/jnr already mounted"
else
  echo "mkfs on /dev/sde3 mount to /mnt/jnr"
  sudo $MKFS /dev/sde3
  sudo mount -t "$1" /dev/sde3 /mnt/jnr
fi

if mountpoint -q /mnt/cache
then
  echo "/mnt/cache already mounted"
else
  echo "mkfs on /dev/sde2 mount to /mnt/cache"
  sudo $MKFS /dev/sde2
  sudo mount -t "$1" /dev/sde2 /mnt/cache
fi
