@echo off
if "%1"=="" goto MISUSE

echo.
echo Searching for Windows setup files...
echo.


:LOOP
if "%1"=="" goto NOTFOUND
if exist %1:\WIN95\OEMSETUP.EXE goto FOUND
shift
goto LOOP


:FOUND
cls
echo.
echo Windows setup files found on drive %0:
echo.
echo Launching setup...
echo.
set PATH=A:;%1:;%1:\WIN95
%1:
cd WIN95
OEMSETUP.EXE /k "A:\DRVCOPY.INF"
goto QUIT


:NOTFOUND
cls
echo.
echo.
echo The Windows setup files could not be found.
echo Make sure your CD contains \WIN95\OEMSETUP.EXE
echo.
goto QUIT

:MISUSE
echo.
echo Usage: RUNSETUP.BAT Z Y X W ...
echo.

:QUIT
