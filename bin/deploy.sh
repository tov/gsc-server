#!/bin/sh

set -eux

cd "$(dirname $0)"/..

BASE=/home/gsc
BUILD_TYPE=release
BUILD_DIR=build.$BUILD_TYPE

install_suid () {
    sudo install -v -o gsc -m 4555 "$@"
}

install_to () {
    rsync \
        --recursive \
        --links --copy-unsafe-links \
        server_root/ \
        "$1"
    chmod -R a+rX "$1"

    install_suid $BUILD_DIR/gscd-fcgi "$1/gscd.fcgi"
    install_suid $BUILD_DIR/gsc-auth "$1/gsc-auth"
}

stage () {
    link=${1:-staging}
    tree=${2:-deploy-$(date +%Y%m%d-%H%M%S)}
    cmake --build $BUILD_DIR
    install_to "$BASE/$tree"
    rm -f "$BASE/$link"
    ln -s "$tree" "$BASE/$link"
}

unstage () {
    link=${1:-staging}
    rm -Rf "$(realpath $BASE/$link/)"
    rm -f "$BASE/$link"
}

deploy () {
    src=${1:-staging}
    dst=${2:-production}
    src_tree=$(realpath "$BASE/$src")
    dst_tree=$(realpath "$BASE/$dst" 2>/dev/null || true)

    rm -f "$BASE/$dst"
    ln -s "$src_tree" "$BASE/$dst"

    if [ -n "$dst_tree" ]; then
        rm -f "$BASE/$src"
        ln -s "$dst_tree" "$BASE/$src"
    fi
}

"$@"
