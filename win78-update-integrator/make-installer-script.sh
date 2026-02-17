#!/bin/sh
set -eu

# usage: make-installer-script update-list-file
listfile="$(readlink -f "$1")"

cd "$(dirname "$(readlink -f "$0")")"

printf '@echo off\r\n'
printf 'cd /d "%%~dp0"\r\n'
printf '\r\n'

lines="$(wc -l "$listfile" | cut -d' ' -f1)"
i=0
while read -r l; do
    l="$(printf '%s\n' "$l" | xargs)"
    i="$((i+1))"

    if [ -z "$l" ] || [ "${l#;}" != "$l" ]; then
        # blank line or ;comment
        printf 'echo (%s/%s) %s\r\n' "$i" "$lines" "$l"
    else
        printf 'echo (%s/%s) Installing %s\r\n' "$i" "$lines" "$l"
        printf 'wusa.exe "%%~dp0%s" /quiet /norestart\r\n' "$l"
        printf 'call :checkerror\r\n'
    fi
done < "$listfile"

printf 'echo.\r\n'
printf 'echo Finished.\r\n'
printf 'exit /b\r\n'
printf '\r\n'
printf ':checkerror\r\n'
printf 'if %%ERRORLEVEL%%==0 (\r\n'
printf '    echo ---^> ok\r\n'
printf '    exit /b\r\n'
printf ')\r\n'
printf 'if %%ERRORLEVEL%%==3010 (\r\n'
printf '    echo ---^> ok, reboot required\r\n'
printf '    exit /b\r\n'
printf ')\r\n'
printf 'if %%ERRORLEVEL%%==2359302 (\r\n'
printf '    echo ---^> already installed\r\n'
printf '    exit /b\r\n'
printf ')\r\n'
printf 'if %%ERRORLEVEL%%==-2145124329 (\r\n'
printf '    echo --^> not applicable\r\n'
printf '    exit /b\r\n'
printf ')\r\n'
printf 'echo. FAILED! (%%ERRORLEVEL%%)\r\n'
printf 'pause\r\n'
printf 'exit /b\r\n'
printf '\r\n'
