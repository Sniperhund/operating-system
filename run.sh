#!/bin/bash
set -e

if [ "$1" == "--symbol" ]; then
    (cd libc && make clean && bear -- make all)
    (cd programs && make clean && bear -- make all)
    (cd kernel && make clean && bear -- make build)
    exit 0
fi

if ! mountpoint -q /mnt/mydisk; then
    (cd kernel && make mount-disk)
fi

(cd libc && make all)
(cd programs && make all)

echo "Moving binaries..."
cp --remove-destination -r ./programs/binaries/* /mnt/mydisk/bin/
sync

if [ "$1" == "--debug" ]; then
    (cd kernel && make run-debug)
else
    (cd kernel && make run)
fi
