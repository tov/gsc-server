#!/bin/sh

cd "$(dirname $0)"/..

mkdir -p build.release
(
cd build.release
cmake -DCMAKE_BUILD_TYPE=Release ..
make gsc-fcgi
)

rm -f server_root/gsc

sudo install -o gsc -m 4555 build.release/gsc-fcgi server_root/gsc
