#!/bin/sh

set -eu

cd "$(dirname "$(readlink -f "$0")")"

printf 'Preparing...\n'

rm -rf tmp
rm -rf iso
mkdir -p tmp
cp -aR iso-orig iso

printf 'Fixing filename casing...\n'
find iso -depth -printf '%f:%h\n' | while IFS=':' read -r name dir; do mv -n -T "$dir/$name" "$dir/$(printf '%s\n' "$name" | tr 'A-Z' 'a-z')"; done

mkdir -p iso/i386/svcpack

# find and extract cab files
find iso/i386 -maxdepth 1 -iname '*.cab' -printf '%f:%p\n' | while IFS=':' read -r cabfile path; do
    printf 'Extracting %s...\n' "$cabfile"
    cabextract -qd"tmp/$cabfile" "$path"
    find "tmp/$cabfile" -depth -printf '%f:%h\n' | while IFS=':' read -r name dir; do mv -n -T "$dir/$name" "$dir/$(printf '%s\n' "$name" | tr 'A-Z' 'a-z')"; done
done

kblist="$(cd updates; for i in *-kb*.exe; do printf '%s\n' "$i" | sed 's/^.*-\(kb[0-9]\+\).*\.exe$/\1/'; done | sort -Vu)"

for kb in $kblist; do
    for file in $(cd updates; ls *-$kb*.exe); do
        ./integ-worker.sh "$kb" "$file"
    done
done

if [ -d extra-files ]; then
    ./integ-worker.sh extra-files
fi

printf 'Building svcpack.inf...\n'
./generate-svcpack.sh

printf 'Merging duplicate files...\n'
rdfind -makeresultsfile false -makehardlinks true iso
