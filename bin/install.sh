#!/bin/sh

cd "$(dirname $0)"/..

mkdir -p build
# make -C build gsc-fcgi
rm -f server_root/gsc-fcgi
cp build/gsc-fcgi server_root/

(
cd server_root
sudo chown gsc:gsc gsc-fcgi
sudo chmod 6555 gsc-fcgi
)
