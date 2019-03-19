#!/bin/sh

cd "$(dirname $0)"/..

# Require password up front
sudo true

bin/build.sh release

git ls-files server_root | xargs chmod a+r

sudo install -v -o gsc -m 4555 \
    build.release/gscd-fcgi server_root/gscd.fcgi
sudo install -v -o gsc -m 4555 \
    build.release/gscauth server_root/gscauth

sudo service apache2 restart
