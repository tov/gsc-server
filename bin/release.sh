#!/bin/sh

cd "$(dirname $0)"/..

bin/build-release.sh

sudo install -o gsc -m 4555 build.release/gsc-fcgi server_root/gsc
