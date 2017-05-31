#!/bin/bash

SVR=servernode
BRK0=/data/gfs0/brick
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
  echo -e "\nUsage - create: `basename $0` \$TYPE create \$M \$N"
  echo -e "  ==> \$TYPE can be dsi or gf, \$M is redundancy (i.e., failure tolerance), \$N is total number of nodes"
  echo -e "  ==> When \$M=2, default number of nodes for dsi EC is 7, and when \$M=3, the default is 8"
  echo -e "Usage - other ops: `basename $0` start | mount | stop | delete | info\n"
  exit
fi

# create DSI EC
if [ "$#" -eq 4 ] && [ "$1" == "dsi" ] && [ "$2" == "create" -o "$2" == "c" ] 
then
  echo "Create $4-node DSI EC tolerating $3 failures"
  cmd="sudo gluster volume create ecvol dsi-ec $4 redundancy $3 "
  for (( i = 1; i <= $4; i++ ))
  do
    var="BRK$i"
    cmd+="$SVR:${!var} "
  done
  cmd+="force"
  echo "$cmd"
  $cmd
  exit
fi

# create original EC
if [ "$#" -eq 4 ] && [ "$1" == "gf" ] && [ "$2" == "create"  -o "$2" == "c" ] 
then
  echo "Create $4-node GlusterFS EC tolerating $3 failures"
  cmd="sudo gluster volume create ecvol redundancy $3 "
  for (( i = 1; i <= $4; i++ ))
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
  sudo gluster volume start ecvol
  ;;
  "mount" | "m")
  sudo mount.glusterfs $SVR:ecvol $MOUNTPATH
  ;;
  "mount4" | "m4")
  sudo mount.glusterfs $SVR:ecvol "$MOUNTPATH"1
  sudo mount.glusterfs $SVR:ecvol "$MOUNTPATH"2
  sudo mount.glusterfs $SVR:ecvol "$MOUNTPATH"3
  sudo mount.glusterfs $SVR:ecvol "$MOUNTPATH"4
  ;;
  "stop" | "sto" )
  sudo umount -l $MOUNTPATH
  sudo umount -l "$MOUNTPATH"1
  sudo umount -l "$MOUNTPATH"2
  sudo umount -l "$MOUNTPATH"3
  sudo umount -l "$MOUNTPATH"4
  sudo gluster --mode=script volume stop ecvol
  ;;
  "delete" | "d")
  sudo gluster --mode=script volume delete ecvol
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

