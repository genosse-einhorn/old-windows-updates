#!/bin/sh

set -eu
cd "$(dirname "$(readlink -f "$0")")"


find update-repo -type f -iname '*.exe' -printf '%f' | egrep -o '[kK][bB][0-9]+' | tr 'A-Z' 'a-z' | sort -V
