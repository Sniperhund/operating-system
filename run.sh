#!/bin/bash
set -e

if ! mountpoint -q /mnt/mydisk; then
    (cd kernel && make mount-disk)
fi

(cd libc && make all)
(cd programs && make all)

echo "Moving binaries..."
cp --remove-destination -r ./programs/binaries/* /mnt/mydisk/bin/
rm -rf ./programs/binaries
sync

(cd kernel && make run)