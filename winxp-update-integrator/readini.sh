#!/bin/sh

set -eu

section=
cr="$(printf '\r')"
tab="$(printf '\t')"
linecont=

sed -E -e 's/\r$//' -e 's/^\s+//' -e 's/\s+$//' | while IFS= read -r l; do
    if [ "${l%\\}" != "$l" ]; then
        linecont="$linecont${l%\\}"
        continue
    fi

    if [ -n "$linecont" ]; then
        l="$linecont$l"
        linecont=
    fi

    if [ -z "$l" ] || [ "${l#;}" != "$l" ]; then
        # comment or blank, ignore
        :
    elif [ "${l#[}" != "$l" ] && [ "${l%]*}" != "$l" ]; then
        # section header
        l="${l#*[}"
        l="${l%]*}"
        section="$l"
    else
        # line, echo it
        printf '%s\t%s\n' "$section" "$l"
    fi
done
