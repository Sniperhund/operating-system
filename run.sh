#!/bin/bash
set -e

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
