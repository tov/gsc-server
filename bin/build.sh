#!/bin/sh

cd "$(dirname $0)"/..

case "$1" in
    debug)
        cmake_type=Debug
        build_dir=build.debug
        target=gsc
        ;;
    release)
        cmake_type=Release
        build_dir=build.release
        target=gsc-fcgi
        ;;
    *)
        echo >&2 "Usage: $0 [ debug | release ]"
        exit 1
        ;;
esac

if [ ! -d $build_dir ]; then
    mkdir $build_dir
    (
    cd $build_dir
    cmake -DCMAKE_BUILD_TYPE=$cmake_type ..
    )
fi

make -j 4 -C $build_dir $target

