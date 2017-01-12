#!/bin/sh

cd "$(dirname $0)"/..

mkdir -p build
make -C build gsc-fcgi

rm -f server_root/gsc-fcgi

sudo install -o gsc -m 4555 build/gsc-fcgi server_root/
