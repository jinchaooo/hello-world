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

# for hot tier bricks
HBRK1=/data/ht1/brick
HBRK2=/data/ht2/brick
HBRK3=/data/ht3/brick
HBRK4=/data/ht4/brick
HBRK5=/data/ht5/brick
HBRK6=/data/ht6/brick

if [ "$#" -lt 1 ]
then
  echo -e "\nUsage - attach hot tier to volume: `basename $0` a \$VOL \$M \$N"
  echo -e "  ==> \$VOL is original (cold) volume, \$M is replica count of hot bricks"
  echo -e "  ==> \$N is total number of hot bricks"
  echo -e "Usage - check tier status: `basename $0` s \$VOL"
  echo -e "Usage - start detach hot tier from volume: `basename $0` d \$VOL start"
  echo -e "Usage - check detach status of volume: `basename $0` d \$VOL status"
  echo -e "Usage - commit detach of volume: `basename $0` d \$VOL commit\n"
  echo -e "Usage - other ops: `basename $0` \$VOL start | mount | stop | delete | info\n"
  exit
fi

# attach default bricks
if [ "$#" -eq 4 ] && [ "$1" == "a" ]
then
  cmd="sudo gluster volume tier $2 attach "
  if [ $3 -gt 1 ]
  then
    cmd+="replica $3 "
  fi
  for (( i = 1; i <= $4; i++ ))
  do
    var="HBRK$i"
    cmd+="$SVR:${!var} "
  done
  cmd+="force"
  echo "$cmd"
  $cmd
  exit
fi

# attach explicit bricks
if [ "$#" -gt 4 ] && [ "$1" == "a" ]
then
  cmd="sudo gluster volume tier $2 attach "
  if [ $3 -gt 1 ]
  then
    cmd+="replica $3 "
  fi
  for (( i = 1; i <= $4; i++ ))
  do
    var=$(( 4 + i ))
    cmd+="${!var} "
  done
  cmd+="force"
  echo "$cmd"
  $cmd
  exit
fi

if [ "$#" -eq 2 ] && [ "$1" == "s" ]
then
  sudo gluster volume tier $2 status
  exit
fi

if [ "$#" -eq 3 ] && [ "$1" == "d" ]
then
  case $3 in 
    "start")
    sudo gluster volume tier $2 detach start
    ;;
    "status")
    sudo gluster volume tier $2 detach status
    ;;
    "commit")
    sudo gluster volume tier $2 detach commit
    ;;
    *)
    echo "invalid operation"
    ;;
  esac
  exit
fi

# other ops
if [ "$#" -eq 2 ]
then
  case "$2" in 
    "start" | "sta")
    sudo gluster volume start $1
    ;;
    "mount" | "m")
    sudo mount.glusterfs $SVR:$1 $MOUNTPATH
    ;;
    "mount4" | "m4")
    sudo mount.glusterfs $SVR:$1 "$MOUNTPATH"1
    sudo mount.glusterfs $SVR:$1 "$MOUNTPATH"2
    sudo mount.glusterfs $SVR:$1 "$MOUNTPATH"3
    sudo mount.glusterfs $SVR:$1 "$MOUNTPATH"4
    ;;
    "stop" | "sto" )
    sudo umount -l $MOUNTPATH
    sudo umount -l "$MOUNTPATH"1
    sudo umount -l "$MOUNTPATH"2
    sudo umount -l "$MOUNTPATH"3
    sudo umount -l "$MOUNTPATH"4
    sudo gluster --mode=script volume stop $1 
    ;;
    "delete" | "d")
    sudo gluster --mode=script volume delete $1
    cmd="sudo rm -fr /data/ssd/* "
    for (( i = 1; i <= 9; i++ ))
    do 
      var="BRK$i"
      cmd+="${!var} "
    done
    for (( i = 1; i <= 6; i++ ))
    do 
      var="HBRK$i"
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
fi

if [ "$#" -eq 1 ] && [ "$1" == "info" -o "$1" == "i" ]
then
  sudo gluster volume info
fi
