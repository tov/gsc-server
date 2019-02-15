#!/bin/sh

cd "$(dirname $0)"/..

case "$1" in
    debug)
        cmake_type=Debug
        build_dir=build.debug
        target=gscd
        ;;
    release)
        cmake_type=Release
        build_dir=build.release
        target=gscd-fcgi
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
chmod a+rx $build_dir/$target
git ls-files server_root | xargs chmod a+r
