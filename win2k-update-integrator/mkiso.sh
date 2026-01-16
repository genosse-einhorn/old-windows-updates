#!/bin/sh

set -eu
cd "$(dirname "$(readlink -f "$0")")"

if [ -e 'iso/[boot]/boot-noemul.img' ]; then
    mv 'iso/[boot]/boot-noemul.img' iso/boot.img
    rm -rf 'iso/[boot]'
fi

if [ -e boot.img ]; then
    cp boot.img iso/boot.img
fi

if [ -e winnt.sif ]; then
    cp winnt.sif iso/i386/winnt.sif
fi

mkisofs -o winmod.iso -V WIN2K -iso-level 3 -J -N -D -b boot.img -no-emul-boot -hide 'boot.*' -hide-joliet 'boot.*' iso
