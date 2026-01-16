#!/bin/sh

set -eu
cd "$(dirname "$(readlink -f "$0")")"

if [ -e 'iso/[BOOT]/Boot-NoEmul.img' ]; then
    mv 'iso/[BOOT]/Boot-NoEmul.img' iso/boot.img
    rm -rf 'iso/[BOOT]'
fi

if [ -e boot.img ]; then
    cp boot.img iso/boot.img
fi

if [ -e winnt.sif ]; then
    cp winnt.sif iso/I386/winnt.sif
fi

mkisofs -o winmod.iso -V WINXP -iso-level 3 -J -N -D -b boot.img -no-emul-boot -hide 'boot.*' -hide-joliet 'boot.*' iso iso-addons
