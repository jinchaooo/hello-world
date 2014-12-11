#!/bin/bash
if [ $# -lt 1 ]
then
  echo "usage: $0 rbd_size(MB) [mon_ip]"
  echo "example: $0 10240 [192.168.35.182]"
  exit 0
fi

if [ $# -eq 1 -a "$1" = "u" ]
then 
  echo "unmap and delete rbd image jrbd"
  mapped=`rbd showmapped`
  devid=`echo $mapped | awk '{for(i=1;i<=NF;i++) if($i=="jrbd") {i=i+2;print $i;break;}}'`
  sudo rbd unmap $devid
  rbd rm jrbd
  exit 0
fi

if [ $# -eq 2 ]
then
  echo "create and map rbd image jrbd"
  rbd create --pool rbd --size $1 --order 22 --image-format 1 --stripe-unit 4194304 --stripe-count 1 -m $2 jrbd 
  sudo rbd --pool rbd -m $2 map jrbd
else
  echo "create and map rbd image jrbd"
  rbd create --pool rbd --size $1 --order 22 --image-format 1 --stripe-unit 4194304 --stripe-count 1 jrbd
  sudo rbd --pool rbd map jrbd
fi
