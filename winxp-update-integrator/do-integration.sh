#!/bin/sh

set -eu
cd "$(dirname "$(readlink -f "$0")")"

# cleanup
rm -rf iso
cp -a orig-iso iso

if [ -d iso/i386 ]; then
    mv iso/i386 iso/I386
fi

# extract sp3 and driver cab
rm -rf tmp

for i in sp3.cab driver.cab mmssetup.cab iis6.cab; do
    find iso/I386 -maxdepth 1 -iname "$i" -exec cabextract -qsd tmp/$i '{}' \;
done

# now integrate them
while read -r kb; do
    ./integrate-update.sh "$kb"
done < kblist.txt

# recompress driver and sp3 cab
for i in $(find tmp -maxdepth 1 -type d -iname '*.cab' -printf '%f\n'); do
    find iso/I386 -maxdepth 1 -iname "$i" -exec gcab -cnz '{}' tmp/$i/* \;
done

# copy addons
if [ -d addons ]; then
    mkdir -p iso/I386/svcpack
    find addons -maxdepth 1 -type f -iname '*.exe' -printf '%f\n' | while read -r f; do
        kbno="$(printf '%s\n' "$f" | egrep -io 'kb[0-9]+' | tr A-Z a-z)"
        if [ -n "$kbno" ]; then
            cp -a "addons/$f" "iso/I386/svcpack/$kbno.exe"
        else
            cp -a "addons/$f" "iso/I386/svcpack/$f"
        fi
    done
fi

# generate svcpack inf
./generate-svcpack.sh
