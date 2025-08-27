# Introduction

Most if not all Windows 95 CDs are not bootable. So how do I install Windows 95 onto a new PC or VM?

* Retail versions of Windows 95 assume you already have DOS with a CD driver installed.
* OEM versions of Windows 95 are supposed to come with a bootdisk that carries CD drivers.

Most OEMs didn't actually bother to put a CD driver on the bootdisk (if they shipped one at all...), and
even if they supplied a driver it likely won't work for the CD drive you currently have.

This document will show you how to modify a Win95 bootdisk to load a CD driver and start Windows 95 setup from CD.

## Ingredients

*   A Windows 95 bootdisk. You can create one from a running Windows 95 installation via `Control Panel` ➡ `Add/Remove Programs` ➡ `Startup Disk`.
*   A real-mode CD-ROM driver. I use the venerable [VIDE-CDD.SYS](https://vogonsdrivers.com/getfile.php?fileid=1456&menustate=0). Another good choice is `OAKCDROM.SYS` from Windows 98.
*   `HIMEM.SYS` from Windows 95 (`C:\WINDOWS\HIMEM.SYS`)
*   `MSCDEX.EXE` from Windows 95 (`C:\WINDOWS\COMMAND\MSCDEX.EXE`)
*   `CONFIG.SYS`, `AUTOEXEC.BAT`, `RUNSETUP.BAT`, `DRVCOPY.INF` made by me

## How To Make

Take the bootdisk and copy the other files onto it.

Modify `CONFIG.SYS` and `DRVCOPY.INF` to point to your chosen CD driver.

## Extras

* The startup disk contains potentially useless stuff like `UNINSTAL.EXE` or `REGEDIT.EXE`.
  If you delete this, you'll have space left for e.g. [patcher9x](https://github.com/JHRobotics/patcher9x)
* Not all CDs contain `OEMSETUP.EXE`, especially not the retail and update CDs.
  If you have such a disk, you'll have to partition and format the drive manually
  and then run `SETUP.EXE /K A:\DRVCOPY.INF` (or just `SETUP.EXE` if you don't want
  the real mode CD driver to be permanently installed).
* `OEMSETUP.EXE` checks the DOS version. A DOS 7.0 bootdisk made by the RTM or OSR1 (A) version of Win95
  can only be used to install Win95 RTM or OSR1. A DOS 7.1 bootdisk made by some OSR2 (B/C) version
  can be used to install Win95 OSR2 only (or Win98 for that matter, but why would you do this when
  the Win98 cd is already bootable)

## Credits

* `DRVCOPY.INF` is basically stolen from some official OEM boot disk.
* The `CONFIG.SYS` menu system is inspired by the bootable Windows 98 CD.




