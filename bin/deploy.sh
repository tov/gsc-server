#!/bin/sh

set -e

cd "$(dirname $0)"/..

for_dirs_upward () (
    local dir
    dir=$(cd "$1"; pwd); shift

    while "$@" "$dir" && [ "$dir" != / ]; do
        dir=$(dirname "$dir")
    done
)

publish_dirs () {
    local dir

    for dir; do
        sudo chmod -R a+rX "$dir"
        for_dirs_upward "$dir" sudo chmod -f a+x
    done
}

gsc_install () {
    sudo install -v -o gsc -m 4555 "$@"
}

if [ "$1" = "-d" ]; then
    build_type=debug
else
    build_type=release
fi

# Require password up front
sudo true

make -C server_root/html
publish_dirs server_root 3rdparty/wt/resources

bin/build.sh $build_type gscd-fcgi gsc-auth
gsc_install build.$build_type/gscd-fcgi server_root/gscd.fcgi
gsc_install build.$build_type/gsc-auth server_root/gsc-auth

sudo service apache2 restart
