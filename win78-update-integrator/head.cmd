cd /d "%~dp0"

IF "%~1"=="" GOTO usage
IF "%~2"=="" GOTO usage
GOTO script

:usage
@echo Usage: %0 wimfile index
@exit /B 1

:script
md c:\wimmount

dism /Mount-Wim /WimFile:%1 /Index:%2 /MountDir:c:\wimmount
if %errorlevel% neq 0 pause
