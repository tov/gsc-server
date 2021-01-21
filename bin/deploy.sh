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
    local cfg="$1"
    local dst="$2"

    rsync \
        --recursive \
        --links --copy-unsafe-links \
        server_root/ \
        "$dst"

    if [ -e "$cfg" ]; then
        cp "$cfg" "$dst/gscd-config.json"
    fi

    chmod -R a+rX "$dst"

    install_suid $BUILD_DIR/gscd-fcgi "$dst/gscd.fcgi"
    install_suid $BUILD_DIR/gsc-auth "$dst/gsc-auth"
}

stage () {
    local link=${1:-staging}
    local cfg=${2:-etc/${link}-config.json}
    local tree=${3:-deploy-$(date +%Y%m%d-%H%M%S)}

    sudo true
    cmake --build $BUILD_DIR
    install_to "$cfg" "$BASE/$tree"
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

sudo systemctl reload-or-restart apache2

