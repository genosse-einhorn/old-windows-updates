# Introduction

Windows 98SE copies its installation files to the hard disk during the first stage of setup.
With this guide, you can modify your Windows 95 Setup Files to do the same.

Why would you want to do this? The Windows 95 Setup needs to access the installation
files after the first reboot to install drivers for detected Plug&Play devices,
but it will not be able to access the setup CD or network share if it would
need Plug&Play drivers to do so which it hasn’t installed yet.

# How it works

We use a custom `MSBATCH.INF` file which copies the setup files to the hard disk
and also sets up the new path in the registry.

The INF file looks like this:

    [version]
    Signature=$Chicago$

    [DestinationDirs]
    CABCPY.copy=10,OPTIONS\CABS

    [Install]
    CopyFiles=CABCPY.copy
    AddReg=CABCPY.addreg

    [CABCPY.copy]
    DELTEMP.COM
    DOSSETUP.BIN
    ... ; all files in the WIN95 directory

    [CABCPY.addreg]
    HKLM,SOFTWARE\Microsoft\Windows\CurrentVersion\Setup,"SourcePath",,"%10%\Options\Cabs"

    [SourceDisksFiles]
    DELTEMP.COM=100,,
    DOSSETUP.BIN=100,,
    ... ; all files

    [SourceDisksNames]
    100="Windows 95 CD-ROM","SETUP.EXE",0

If you already use a `MSBATCH.INF` file for setup automation, you can
merge the sections into your old file.

# Generate the INF file

Here is a batch file to generate the `MSBATCH.INF`:

    @echo off
    echo [Version]
    echo Signature=$Chicago$
    echo.
    echo [DestinationDirs]
    echo CABCPY.copy=10,OPTIONS\CABS
    echo.
    echo [Install]
    echo CopyFiles=CABCPY.copy
    echo AddReg=CABCPY.addreg
    echo.
    echo [CABCPY.copy]
    for %%a in (*.*) do echo %%a
    echo.
    echo [CABCPY.addreg]
    echo HKLM,SOFTWARE\Microsoft\Windows\CurrentVersion\Setup,"SourcePath",,"%%10%%\Options\Cabs"
    echo.
    echo [SourceDisksFiles]
    for %%a in (*.*) do echo %%a=100,,
    echo.
    echo [SourceDisksNames]
    echo 100="Windows 95 CD-ROM","SETUP.EXE",0

Save as `X:\Somewhere\mkinf.bat`, open a command prompt inside the `\WIN95` directory
of your setup CD, then run it like `X:\Somewhere\mkinf.bat >MSBATCH.INF`.

Note that the batch file does not deal with subdirectories inside `\WIN95`.
Only some OEM discs contain subdirectories with Internet Explorer stuff, which
can usually be deleted without too many problems.
