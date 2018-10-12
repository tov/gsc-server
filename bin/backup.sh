#!/bin/sh

filename=$(date +%Y%m%d-%H%M%S).bz2
host=login.eecs.northwestern.edu
dir=gsc-backup

rm -f $filename
sudo -u gsc pg_dump gsc | bzip2 > $filename

ssh $host mkdir -p $dir
scp $filename $host:$dir/

rm -f $filename
