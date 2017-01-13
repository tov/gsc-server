#!/bin/sh

cd "$(dirname $0)"/..

mkdir -p build.debug
(
cd build.debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make gsc
)
