# Make upper memory available

You need to use EMM386. Since https://www.virtualbox.org/ticket/10022 has been fixed, you can add the following to `CONFIG.SYS`

    DEVICE=C:\DOS\EMM386 RAM AUTO

If DOS does not detect available upper memory blocks (maybe because the bug returned), you can specify available memory regions manually. Add to `CONFIG.SYS`:

    DEVICE=C:\DOS\EMM386.EXE I=D800-EFFF

# DOS eats all available CPU time for lunch

You can enable the power manager `POWER.EXE` in `CONFIG.SYS`

    DEVICE=C:\DOS\POWER.EXE

# Network

TODO: Document usage of the MS Network client

