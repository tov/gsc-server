#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset

OS=$(uname)
NPROC=1
if [[ $OS = "Linux" ]] ; then
    NPROC=$(nproc)
elif [[ ${OS} = "Darwin" ]] ; then
    NPROC=$(sysctl -n hw.ncpu)
fi

if type clang-format-10 2> /dev/null ; then
    CLANG_FORMAT=clang-format-10
elif type clang-format 2> /dev/null ; then
    CLANG_FORMAT=clang-format
    VER=$(clang-format --version)
    if [[ $VER != *10.0* ]] ; then
        echo "clang-format is not 10.0. (returned ${V})"
        echo "required clang-format version 10.0"
        exit 1
    fi
else
    echo "clang-format not found, (expected clang-format-10, or clang-format version 10.0)"
    exit 1
fi

find src -type f -name '*.C' -o -name '*.h' | xargs -I{} -P ${NPROC} bash -c "${CLANG_FORMAT} -i -style=file {}"
