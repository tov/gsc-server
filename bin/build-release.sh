#!/bin/sh

cd "$(dirname $0)"/..

mkdir -p build.release
(
cd build.release
cmake -DCMAKE_BUILD_TYPE=Release ..
make gsc-fcgi
)
