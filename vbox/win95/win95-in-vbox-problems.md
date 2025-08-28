This document describes some common problems and their solutions when running Windows 95 in VirtualBox.

It was tested with **VirtualBox 7.0** against the Windows 95 versions I have lying around.

# Can't boot - Windows Protection Error while initializing IOS, NDIS, ...

![](screenshots/windows-protection-error-ios.png)

**The emulated CPU is too fast for Windows 95.**

That has been a problem on real hardware in the past too, so microsoft *did* release an update
for Win95 OSR2 (B/C) version known as the **AMD K6 Update** (`amdk6upd.exe` sha256sum: 664f3b087e51a81ce25eba239e64ac37f178340d41ffa1f090ac816f89b6165d). However, this update
is not enough for current CPUs and also pretty much impossible to install nowadays
because the installer needs to be run from within Windows.

A newer patch package is [FIX95CPU](http://lonecrusader.x10host.com/fix95cpu.html) (`FIX95CPU_V3_FINAL.ZIP`, sha256sum 7618245fcc1005a975006c5f625ebf581a58309491642c8fdc9eef583deb0d40), which also works
on the RTM an OSR1 (A) version.

The most up to date patch is [patcher9x](https://github.com/JHRobotics/patcher9x),
which also fixes a TLB update bug on newer AMD Ryzen processors.

# Windows 95 Setup can't find the setup files during device installation

![](screenshots/setup-could-not-find-file.png)

### You're installing from floppies

You probably fell victim to the *4 floppy drives bug* described below. To continue with setup:

* try other drive letters (`D:`, `E:`, `F:`, `G:`) and see if the floppy is recognized there.
* if that doesn't work, skip the file and see below for instructions on how to fix the fallout.

### You're installing from CD

Windows 95 Setup can only access the CD during the second stage of setup if you
used a `DRVCOPY.INF` file to install a real-mode CD-ROM driver. See my `setup-bootdisk-with-cdrom`
tutorial for creating a setup bootdisk which gets it right.

Your only option now is to skip the file and then fix the fallout later (see below).

### Alternative: Copy everything to the hard disk

You can work around this problem by copying the files from the `WIN95` onto your
hard disk and install from there. It is also possible to patch the setup files
to do that for you, see [win95-setup-copy-cabs-to-hdd.md](win95-setup-copy-cabs-to-hdd.md).

# Windows 95 complains about missing components and other breakage caused by skipping files during setup

![](screenshots/boot-cannot-find-file.png)

This can be fixed by reinstalling the affected device drivers.

* Make sure that the Windows 95 setup files (CD or floppies) can be accessed by Windows.
* Go into the *Device Manager* (right-click on `My Computer`, `Properties`) and remove
  every device with a yellow exclamation mark next to it. For network related stuff,
  remove every network card.
* Reboot.

After the reboot, Windows 95 will detect the removed devices and install the drivers from the Windows 95 CD or floppies.

# Windows 95 shows 4 floppy drives, or one floppy drive which won't work

![](screenshots/4-floppies.png)

The Windows 95 floppy driver doesn't quite like the emulated floppy drive.
The easiest solution is to go into the device manager (right-click on `My Computer`, `Properties`)
and remove the floppy controller. Then reboot.

Windows 95 will then access the floppy drive using MS-DOS compatibility mode,
which is potentially much slower but works just fine for our purpose.

# No audio output

Make sure you emulate a SoundBlaster 16 with the default settings.

Launch the `Add New Hardware` wizard in the Control Panel, it will detect the audio card and install drivers for it. Note that the *4 floppy drives bug* might return since the `Add New Hardware` wizard will detect the floppy controller, too. See above.

Alternatively, you can choose the ICH AC97 card and use [a vendor driver](https://archive.org/details/ac97_362) for it.

# I want higher screen resolution and more than 16 colors

Higher screen resolutions are possible with Michal Necasek's [boxv9x driver](https://www.os2museum.com/wp/windows-9x-video-minidriver-hd/).

* Install via *Control Panel*, *Display*, *Settings*, *Advanced Properties*, *Adapter*, *Change...*, *Have Disk...*
* Make sure to **increase the amount of video RAM** in the virtual machine settings to **at least 64MB**, otherwise the screen will stay black
* If Windows 95 requests you to specify a monitor, choose *Plug and Play Monitor*

