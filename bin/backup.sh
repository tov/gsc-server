#!/bin/sh

set -e

dir=gsc-backups
localdir=$HOME/$dir
filename=cs211-$(date +%Y%m%d-%H%M%S)
host=login.eecs.northwestern.edu
gpg_recipient=jesse.tov@gmail.com
gpg_public_key=0xC5668BA5047AE6BD

gpg_encrypt () {
    gpg --recipient "$gpg_recipient" \
        --trusted-key "$gpg_public_key" \
        --enable-progress-filter \
        --encrypt "$1"
}

mkdir -p $localdir
cd $localdir

echo>&2 Dumping database:
sudo -u gsc pg_dump gsc > $filename.psql
echo>&2 Encrypting:
gpg --sign -z 5 --encrypt --recipient "$gpg_recipient" \
    --output $filename.psql.bz2.gpg $filename.psql
# echo>&2 Compressing:
# bzip2 -v $filename.psql
# echo>&2 Encrypting:
# gpg --encrypt 
rm $filename.psql

echo>&2 Synchronizing:
rsync --progress -av $localdir $host:
