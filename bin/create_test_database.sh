#!/bin/sh

cd "$(dirname "$0")/.."
datadir=server_root/var/postgres

if [[ "$1" == -c ]]; then
    pg_ctl -D "$datadir" stop
    rm -Rf "$datadir"
    initdb "$datadir"
    pg_ctl -D "$datadir" start
else
    dropdb gsc
fi

createdb gsc
