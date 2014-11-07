#!/bin/bash
if [ "$#" -lt 2 ]
then
  echo "usage: $0 rbd_size(MB) mon_ip"
  echo "example: $0 10240 192.168.35.182"
  exit 0
fi
rbd create foo --size $1 -m $2 -k /etc/ceph/ceph.client.admin.keyring
rbd map foo --pool rbd --name client.admin -m $2 -k /etc/ceph/ceph.client.admin.keyring