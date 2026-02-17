#!/bin/sh
set -eu

# usage: make-integrator-script update-list-file
listfile="$(readlink -f "$1")"

cd "$(dirname "$(readlink -f "$0")")"

cat head.cmd

lines="$(wc -l "$listfile" | cut -d' ' -f1)"
i=0
while read -r l; do
    l="$(printf '%s\n' "$l" | xargs)"
    i="$((i+1))"

    if [ -z "$l" ] || [ "${l#;}" != "$l" ]; then
        # blank line or ;comment
        printf 'rem (%s/%s) %s\r\n' "$i" "$lines" "$l"
    else
        printf '@echo (%s/%s) Integrating %s\r\n' "$i" "$lines" "$l"
        printf 'dism /Image:C:\wimmount /Add-Package /PackagePath:%s\r\n' "$l"
        printf 'if %%errorlevel%% neq 0 pause\r\n'
    fi
done < "$listfile"

cat foot.cmd
