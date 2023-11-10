#!/usr/bin/env bash

CFLAGS='-Wall -Werror -std=c99 -pedantic -ggdb'

set -xe

mkdir -p bin
gcc main.c -o bin/moffset $CFLAGS

{ set +xe; } &> /dev/null
if [[ "$1" == "run" ]]; then
    echo "=== RUNNING ==="
    ./bin/moffset
fi
