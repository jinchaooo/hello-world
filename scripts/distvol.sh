#!/bin/bash

SVR=servernode
BRK1=/data/gfs1/brick
BRK2=/data/gfs2/brick
BRK3=/data/gfs3/brick
BRK4=/data/gfs4/brick
BRK5=/data/gfs5/brick
BRK6=/data/gfs6/brick
BRK7=/data/gfs7/brick
BRK8=/data/gfs8/brick
BRK9=/data/gfs9/brick
MOUNTPATH=/mnt/gfclient

if [ "$#" -lt 1 ]
then
  echo -e "\nUsage - create: `basename $0` create \$M \$N"
  echo -e "  ==> \$M is replica count, \$N is total number of nodes"
  echo -e "Usage - other ops: `basename $0` start | mount | stop | delete | info\n"
  exit
fi

# create original dist-rep vol (with ADCache if ssd mounted)
if [ "$#" -eq 3 ] && [ "$1" == "create"  -o "$1" == "c" ] 
then
  if [ $[$3 % $2] -ne 0 ]
  then
    echo "Wrong parameters to create dist-rep vol"
    exit
  fi
  echo "Create $3-node GlusterFS dist-rep with replica count $2"
  ssh $SVR "if grep -qs /data/ssd /proc/mounts;then echo \"ADCache enabled\";else echo \"ADCache Disabled\";fi" | cat
  cmd="sudo gluster volume create distvol "
  if [ "$2" -gt 1 ]
  then
    cmd+="replica $2 "
  fi
  for (( i = 1; i <= $3; i++ ))
  do
    var="BRK$i"
    cmd+="$SVR:${!var} "
  done
  cmd+="force"
  echo "$cmd"
  $cmd
  exit
fi

# other ops
case "$1" in 
  "start" | "sta")
  sudo gluster volume start distvol
  ;;
  "mount" | "m")
  sudo mount.glusterfs $SVR:distvol $MOUNTPATH
  ;;
  "mount4" | "m4")
  sudo mount.glusterfs $SVR:distvol "$MOUNTPATH"1
  sudo mount.glusterfs $SVR:distvol "$MOUNTPATH"2
  sudo mount.glusterfs $SVR:distvol "$MOUNTPATH"3
  sudo mount.glusterfs $SVR:distvol "$MOUNTPATH"4
  ;;
  "stop" | "sto" )
  sudo umount -l $MOUNTPATH
  sudo umount -l "$MOUNTPATH"1
  sudo umount -l "$MOUNTPATH"2
  sudo umount -l "$MOUNTPATH"3
  sudo umount -l "$MOUNTPATH"4
  sudo gluster --mode=script volume stop distvol
  ;;
  "delete" | "d")
  sudo gluster --mode=script volume delete distvol
  cmd="sudo rm -fr /data/ssd/* "
  for (( i = 1; i <= 9; i++ ))
  do 
    var="BRK$i"
    cmd+="${!var} "
  done
  ssh $SVR "$cmd"
  ssh $SVR "sudo rm -f /var/log/glusterfs/bricks/*"
  ssh $SVR "sudo rm -f /var/log/glusterfs/*.log /var/log/glusterfs/*.log.*"
  ;; 
  "info" | "i")
  sudo gluster volume info
  ;;
  *)
  echo "invalid operation"
  ;;
esac

