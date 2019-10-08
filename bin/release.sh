#!/bin/sh

cd "$(dirname $0)"/..

# Require password up front
sudo true

bin/build.sh release gscd-fcgi gsc-auth

git ls-files server_root | xargs chmod a+r

sudo install -v -o gsc -m 4555 \
    build.release/gscd-fcgi server_root/gscd.fcgi
sudo install -v -o gsc -m 4555 \
    build.release/gsc-auth server_root/gsc-auth

sudo service apache2 restart
