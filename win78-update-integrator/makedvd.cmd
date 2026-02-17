cd /d "%~dp0"

del /f /q dvd\sources\ei.cfg

oscdimg\oscdimg -u2 -m -bootdata:2#p0,e,boscdimg\etfsboot.com#pEF,e,boscdimg\efisys.bin dvd dvd.iso
if %errorlevel% neq 0 pause
