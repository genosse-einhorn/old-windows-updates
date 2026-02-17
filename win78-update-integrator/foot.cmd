
dism /Image:C:\wimmount /Cleanup-Image /StartComponentCleanup
if %errorlevel% neq 0 pause
dism /Image:C:\wimmount /Cleanup-Image /StartComponentCleanup /ResetBase
if %errorlevel% neq 0 pause

rem md c:\wimmount\windows\setup\Scripts
rem if %errorlevel% neq 0 pause
rem copy /y updates\Scripts\*.* c:\wimmount\windows\setup\Scripts
rem if %errorlevel% neq 0 pause

dism /Unmount-Wim /MountDir:c:\wimmount /Commit
if %errorlevel% neq 0 pause
