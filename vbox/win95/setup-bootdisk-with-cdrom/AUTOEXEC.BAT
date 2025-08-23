@ECHO OFF
set EXPAND=YES
SET DIRCMD=/O:N
set temp=c:\
set tmp=c:\
path=a:\

IF "%config%"=="NOCD" GOTO QUIT
LH MSCDEX.EXE /D:oemcd001 /L:D
echo.

IF "%config%"=="CD" GOTO QUIT

COMMAND.COM /F /C RUNSETUP.BAT Z Y X W V U T S R Q P O N M L K J I H G F E D

:QUIT
