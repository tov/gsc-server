#!/bin/sh

cd "$(dirname $0)/.."

${dir:-build.debug}/gscd \
    --docroot server_root/html \
    --approot server_root \
    --http-listen 0.0.0.0:9090
