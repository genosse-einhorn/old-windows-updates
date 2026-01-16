#!/bin/sh

set -eu
cd "$(dirname "$(readlink -f "$0")")"

rm -rf updates-extracted
mkdir -p updates-extracted
while read -r kb; do
    printf '%s\n' "$kb"
    find update-repo -iname "*-$kb*.exe" -exec wine \{\} "/x:$(winepath -w "$PWD/updates-extracted/$kb")" /quiet \;

    # fixup casing of folders and files
    find updates-extracted/$kb -depth -printf '%f:%h\n' | while IFS=':' read -r name dir; do mv -n -T "$dir/$name" "$dir/$(printf '%s\n' "$name" | tr 'A-Z' 'a-z')"; done
done < kblist.txt
