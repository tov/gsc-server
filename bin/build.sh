#!/bin/sh

cd "${0%/*}"
. ./setup-build-env.sh
cd ..

case "$1" in
    debug)
        cmake_type=Debug
        target=
        ;;
    release)
        cmake_type=Release
        target=gscd-fcgi
        ;;
    *)
        echo >&2 "Usage: $0 [ debug | release ]"
        exit 1
        ;;
esac

build_dir=build.$1

if [ ! -d $build_dir ]; then
    mkdir $build_dir
    (
    cd $build_dir
    cmake -DCMAKE_BUILD_TYPE=$cmake_type ..
    )
fi

make -j 4 -C $build_dir $target gscauth
