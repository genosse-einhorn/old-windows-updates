# DOS Mode in Windows Me

IO.SYS and COMMAND.COM from network installation kit:

* `\Tools\NetTools\FAC\cbs.dta\winboot.sys` ➡️ `C:\IO.sys`
* `\Tools\NetTools\FAC\ltools.dta\command.com` ➡️ `C:\command.com` and `C:\WINDOWS\command.com`

modify `C:\WINDOWS\REGENV32.EXE` in hex editor, replace `:\AUTOEXEC.BAT` and `:\CONFIG.SYS` strings with dummies





