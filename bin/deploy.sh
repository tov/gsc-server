#!/bin/sh

cd "$(dirname $0)"/..

for_dirs_upward () (
    local dir
    dir=$1; shift

    dir=$(cd "$dir"; pwd)

    while true; do
        "$@" "$dir" || break
        test "$dir" = / && break
        dir=$(dirname "$dir")
    done
)

publish_dirs () {
    local dir

    for dir; do
        find "$dir" | xargs chmod -f a+rX
        for_dirs_upward "$dir" chmod -f a+x
    done
}

set -e

# Require password up front
sudo true

bin/build.sh release gscd-fcgi gsc-auth

publish_dirs server_root 3rdparty/wt/resources

sudo install -v -o gsc -m 4555 \
    build.release/gscd-fcgi server_root/gscd.fcgi
sudo install -v -o gsc -m 4555 \
    build.release/gsc-auth server_root/gsc-auth

sudo service apache2 restart
