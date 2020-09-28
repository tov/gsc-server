#!/bin/sh

set -eu

cd "$(dirname $0)"/..

case $# in
    (0) set -- build.debug build cmake-build-debug;;
    (1) ;;
    (*) echo>&2 "Usage: $0 [BUILD_DIR]"
        exit 1;;
esac

go () {
    cd server_root
    ../$1/gsc-createdb -c
    exit
}

# Drop and recreate the database
dropdb gsc
createdb gsc

# Need gsc-createdb to create the tables.
for dir; do
    if [ -x $dir/gsc-createdb ]; then
        go $dir
    fi
done

bin/build.sh debug gsc-createdb &&
    go build.debug
