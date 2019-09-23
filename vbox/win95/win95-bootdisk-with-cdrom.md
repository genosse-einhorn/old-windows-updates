# Introduction

How do I install Windows 95 onto a new PC or VM?
Most if not all Windows 95 CDs are not bootable.
Retail versions of Windows 95 assume you already have DOS or Windows 3.1 with a CD driver installed.
OEM versions of Windows 95 are supposed to come with a bootdisk that carries CD drivers.

Most OEMs didn't actually bother to put a CD driver on the bootdisk (if they shipped one at all...), and
even if they supplied a driver it likely won't work for the CD drive you currently have.

This document will show you how to modify a Win95 bootdisk to load a CD driver and start Windows 95 setup from CD.

Ingredients:

*   A Windows 95 bootdisk. You can create one from a running Windows 95 installation via `Control Panel` ➡ `Add/Remove Programs` ➡ `Startup Disk`.
*   A CD-ROM driver. I use `OAKCDROM.SYS` from Windows 98, which works with about every IDE/ATAPI CD drive.
*   `MSCDEX.EXE` from Windows 95 (`C:\WINDOWS\COMMAND\MSCDEX.EXE`) or Windows 98.

# CD-ROM support for Win95 bootdisk

Copy `OAKCDROM.SYS` and `MSCDEX.EXE` to the bootdisk

add to `CONFIG.SYS`:

    device=oakcdrom.sys /D:oemcd001
    lastdrive=z

create `AUTOEXEC.BAT`:

    @ECHO OFF
    LH MSCDEX.EXE /D:oemcd001 /L:D

done!

# Automatically start Win95 setup from CD-ROM

Create `FIND-CD.COM` using the following batch file

    @echo off
    echo >$ n FIND-CD.COM
    echo >>$ e100 E8 "P"0 BA 96 2 B4 9 CD "!1"C9 B8 B 15 "1"DB CD "/"81
    echo >>$ e114 FB AD AD "u"B 9 C0 "u"19 "A"81 F9 1A 0 "r"E8 BA A4 2 B4
    echo >>$ e128 9 CD "!"BD B9 2 E8 D3 0 B8 1 "L"CD "!"80 C1 "A"88 E B4
    echo >>$ e13C 2 88 E BF 2 BA B1 2 B4 9 CD "!"BD B9 2 E8 DC 0 B8 0 "L"
    echo >>$ e151 CD "!"8B 1E 16 0 83 FB 8 "vY"8E C3 "&"A1 0 0 "="CD " u"
    echo >>$ e166 "N&"A1 ","0 "H"8E C0 40 "&"8A E 0 0 80 F9 "Mt"5 80 F9
    echo >>$ e17B "Zu7&;"1E 1 0 "u0&"8B 1E 3 0 83 FB 2 "r&"F6 C7 F0 "u!"
    echo >>$ e194 B1 4 D3 E3 89 1E "^"0 A3 "\"0 8E C0 BE FF FF "F&"83 "<"
    echo >>$ e1A8 0 "u"F9 "FF)"F3 89 1E "`"0 F8 C3 "1"C0 A3 "\"0 A3 "^"0
    echo >>$ e1BD A3 "`"0 F9 C3 A1 "\"0 83 F8 0 "t4"8E C0 "1"DB "&9"1F 74
    echo >>$ e1D2 "+1"F6 8A 2 "&:"0 "u"11 "F<=u"F4 "&"80 "8"0 "t"3 "F"EB
    echo >>$ e1E9 F7 "F"F8 C3 "&"8B 0 "F<"0 "u"F8 80 FC 0 "t"4 1 F3 EB D5
    echo >>$ e1FE "1"DB 89 DE F9 C3 E8 BB FF "r"1F 1 "6`"0 "&"80 "8"0 "u"
    echo >>$ e212 7 "&"C7 7 0 0 F8 C3 "&"8B 0 "&"89 7 "C"83 F8 0 "u"F4 F8
    echo >>$ e227 C3 F9 C3 A1 "\"0 83 F8 0 "tb"8E C0 "1"F6 89 F2 8A 2 "F"
    echo >>$ e23B "<=u"6 9 D2 "u"2 89 F2 "<"0 "u"EF 9 D2 "tGB9"F2 "t"B2
    echo >>$ e252 89 F1 E8 "k"FF A1 "`"0 "r"2 1 F0 "9"C1 "w2"E8 9F FF "1"
    echo >>$ e266 DB "&9"1F "u"8 A1 "`"0 ")"C8 "H"EB C 8B 1E "^"0 A1 "`"0
    echo >>$ e27B ")"C3 "K)"C8 A3 "`"0 "1"F6 8A 2 "&"88 0 "F<"0 "u"F6 "&"
    echo >>$ e290 88 0 F8 C3 F9 C3 "CD-ROM drive $not found."D A "$is A:"
    echo >>$ e2B6 D A "$CDROM=A:"0
    echo >>$ rCX
    echo >>$ 1C2
    echo >>$ w
    echo >>$ q
    debug <$
    del $

Credit: https://www.robvanderwoude.com/amb_cdrom.php

This will give you `FIND-CD.COM` (sha25sum: f733bc12a3b65e393a51e288a38ce799529f0f750eda3044262f94a61baec848).

Then add to `AUTOEXEC.BAT`:

    echo.
    echo Searching CD-ROM Drive...
    FIND-CD

    IF "%CDROM%"=="" GOTO NOCDROM
    IF EXIST "%CDROM%\WIN95\OEMSETUP.EXE" GOTO CDROM
    GOTO NOCDROM

    :CDROM
    echo.
    SET PATH=A:;%CDROM%;%CDROM%\WIN95
    %CDROM%
    cd WIN95
    %CDROM%\WIN95\OEMSETUP.EXE /K DRVCOPY.INF
    goto QUIT

    :NOCDROM
    echo.
    echo The Windows 95 setup files could not be found.
    echo.

    :QUIT

Also create `DRVCOPY.INF`:

    [version]
    signature=$chicago$

    [DestinationDirs]
    RM.driver.dest.copy=30

    [install]
    copyfiles=RM.driver.dest.copy
    updatecfgsys=RM.Sys.upd
    updateautobat=RM.Auto.upd

    [RM.driver.dest.copy]
    OAKCDROM.SYS

    [RM.Sys.upd]
    DevAddDev=OAKCDROM.SYS,device,,"/D:OEMCD001"

    [RM.Auto.upd]
    CmdAdd=MSCDEX.EXE,"/D:OEMCD001 /L:D"

    [SourceDisksNames]
    100="%oem.boot.desc%",,5,A:\

    [SourceDisksFiles]
    OAKCDROM.SYS=100,,2000

    [Strings]
    oem.boot.desc="Windows 95 Setup Bootdisk"

Note that the real-mode CD driver will be permanently installed. This allows you to access the CD drive during the 2nd stage of setup, when rebooting to MS-DOS mode and when launching an application in MS-DOS mode.

# Create a boot menu like Windows 98

This is my `CONFIG.SYS`:

    [Menu]
    MENUITEM=SETUP, Start Windows 95 Setup from CD-ROM
    MENUITEM=CD,    Start DOS with CD-ROM drivers
    MENUITEM=NOCD,  Start DOS without CD-ROM drivers

    [Common]
    DOS=HIGH,UMB
    DEVICE=HIMEM.SYS
    LASTDRIVE=Z

    [SETUP]
    DEVICE=OAKCDROM.SYS /D:OEMCD001

    [CD]
    DEVICE=OAKCDROM.SYS /D:OEMCD001


    [NOCD]
    rem nothing special

and my `AUTOEXEC.BAT`:

    @ECHO OFF

    IF "%CONFIG%"=="NOCD" GOTO QUIT

    LH MSCDEX.EXE /D:oemcd001 /L:D

    echo.
    echo Searching CD-ROM Drive...
    FIND-CD

    IF "%CONFIG%"=="CD" GOTO QUIT

    IF "%CDROM%"=="" GOTO SETUPNOTFOUND
    IF EXIST "%CDROM%\WIN95\OEMSETUP.EXE" GOTO SETUPFOUND
    GOTO SETUPNOTFOUND

    :SETUPFOUND
    echo.
    SET PATH=A:;%CDROM%;%CDROM%\WIN95
    %CDROM%
    cd WIN95
    %CDROM%\WIN95\OEMSETUP.EXE /K A:\DRVCOPY.INF
    goto QUIT

    :SETUPNOTFOUND
    echo.
    echo The Windows 95 setup files could not be found.
    echo.

    :QUIT
