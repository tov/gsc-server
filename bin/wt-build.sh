#!/bin/sh

. "${0%/*}/setup-build-env.sh"

set -e

sudo true

build_dir=build.release

cmake -S . -B $build_dir \
    -DCONNECTOR_HTTP=Off \
    -DBUILD_EXAMPLES=Off \
    -DBUILD_TESTS=Off \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=17

cmake --build $build_dir -- -j 4

sudo cmake --build $build_dir --target install
