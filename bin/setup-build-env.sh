#!/bin/sh

unset CC
unset CXX
unset CFLAGS
unset CXXFLAGS

case "$(hostname)" in
    stewie)
        CC=gcc-8
        CXX=g++-8
        ;;
esac

export CC
export CXX
export CFLAGS
export CXXFLAGS

