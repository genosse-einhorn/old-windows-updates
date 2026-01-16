#!/bin/sh
set -eu

# generate .bat file to install infs
(
    printf '@title Hotfix Reg Installer\r\n'
    find iso/i386/svcpack -maxdepth 1 -type f -iname '*.inf' -printf 'rundll32.exe advpack.dll,LaunchINFSectionEx %%~dp0%P,DefaultInstall,,4,N\r\n' | sort -Vu
) > iso/i386/svcpack/hfreg.cmd

# rewrite svcpack.inf
find iso/i386 -maxdepth 1 -iname 'svcpack.inf' -delete
find iso/i386 -maxdepth 1 -iname 'svcpack.in_' -delete
(
    printf '[CatalogHeader]\r\n'
    printf '\r\n'
    printf '[Version]\r\n'
    printf 'BuildNumber=2195\r\n'
    printf 'MinorVersion=0\r\n'
    printf 'MajorVersion=5\r\n'
    printf 'Signature="$WINDOWS NT$"\r\n'
    printf '[SetupData]\r\n'
    printf 'CatalogSubDir="\i386\svcpack"\r\n'
    printf '[SetupHotfixesToRun]\r\n'
    printf 'hfreg.cmd\r\n'
    find iso/i386/svcpack -maxdepth 1 -type d -iname 'kb*' -printf '%f\\update\\update.exe /q /n /z\r\n'
    printf '[ProductCatalogsToInstall]\r\n'
    find iso/i386/svcpack -maxdepth 1 -iname '*.cat' -printf '%P\r\n'
) > iso/i386/svcpack.inf

