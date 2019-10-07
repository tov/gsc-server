#!/bin/sh

. "${0%/*}/setup-build-env.sh"

build_dir=build.release

if [ ! -d $build_dir ]; then
    mkdir $build_dir
    (
    cd $build_dir
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 ..
    )
fi

cmake --build $build_dir -- -j4

