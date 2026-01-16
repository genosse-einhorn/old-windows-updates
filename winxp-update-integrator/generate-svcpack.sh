#!/bin/sh

set -eu
cd "$(dirname "$(readlink -f "$0")")"

# generate cmd file to install hotfix infs
find iso/I386/svcpack -maxdepth 1 -type f -iname '*.inf' -printf 'rundll32.exe advpack.dll,LaunchINFSectionEx %%~dp0%P,DefaultInstall,,4,N\r\n' | sort -Vu > iso/I386/svcpack/kbreg.cmd

# generate actual svcpack.inf
find iso/I386 -iname 'svcpack.inf' -delete
find iso/I386 -iname 'svcpack.in_' -delete
(
    printf '[Version]\r\n'
    printf 'Signature="$Windows NT$"\r\n'
    printf 'MajorVersion=5\r\n'
    printf 'MinorVersion=1\r\n'
    printf 'BuildNumber=2600\r\n'
    printf '[SetupData]\r\n'
    printf 'CatalogSubDir="\i386\svcpack"\r\n'
    printf '[ProductCatalogsToInstall]\r\n'
    find iso/I386/svcpack -type f -iname '*.cat' -printf '%P\r\n' | sort -Vu
    printf '[SetupHotfixesToRun]\r\n'
    printf 'kbreg.cmd\r\n'
    find iso/I386/svcpack -maxdepth 1 -type f -iname '*.exe' -printf '%P\n' | sort -Vu | while read -r f; do
        flower="$(printf '%s\n' "$f" | tr 'A-Z' 'a-z')"
        if [ "${flower#windowsupdateagent}" != "$flower" ]; then
            printf '%s /wuforce /quiet /norestart\r\n' "$f"
        elif [ "$(exiftool -q -q -p '$FileDescription' "iso/I386/svcpack/$f" | xargs)" = "Win32 Cabinet Self-Extractor" ]; then
            printf '%s /q:a /r:n\r\n' "$f"
        else
            printf '%s /q /n /z\r\n' "$f"
        fi
    done
) > iso/I386/svcpack.inf
