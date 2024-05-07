#!/usr/bin/env python3

from argparse import ArgumentParser, FileType
import re
import os

ap = ArgumentParser()
ap.add_argument('-o', '--output', metavar='OUTFILE.BAT', type=FileType('wb'), default='-')
ap.add_argument('--with-reboot', action='store_true', default=False)
ap.add_argument('inlst', type=FileType('r'))

args = ap.parse_args()

encoding = 'ascii'

def bat(line):
    args.output.write(line.encode(encoding))
    args.output.write(b'\r\n')

def kbno(s):
    m = re.search('[kK][bB]([0-9]+)', s)
    if m is not None:
        return 'KB{}'.format(m.group(1))
    else:
        return None

# print bat header
bat('@ECHO OFF')

cl = []

for l in args.inlst:
    l = l.strip()

    if len(l) == 0:
        # empty line
        continue

    if l.startswith(';'):
        # comment line
        continue

    # line with file to install
    f, a, t = (l + '\t\t\t').split('\t', 2)

    if f == '!CAPTION':
        bat('TITLE ' + (a + ' ' + t).strip())
        continue
    if f == '!CODEPAGE':
        bat('CHCP {} >NUL'.format((a + ' ' + t).strip()))
        encoding = 'cp' + (a + ' ' + t).strip()
        continue

    if f.upper().endswith('.MSU'):
        c = 'START /W WUSA.EXE "%~dp0{}" {}'.format(f, a or '/quiet /norestart')
    elif f.upper().endswith('.CAB'):
        c = 'DISM.EXE {} /online /add-package /packagepath:"%~dp0{}"'.format(a or '/quiet /norestart', f)
    elif f.upper().endswith('.REG'):
        c = 'START /W "" REGEDIT.EXE /s "%~dp0{}" {}'.format(f, a)
    elif f.upper().endswith('.MSI'):
        c = 'START /W "" MSIEXEC.EXE /i "%~dp0{}" {}'.format(f, a or '/qn /norestart')
    elif f.upper().endswith('.MSP'):
        c = 'START /W "" MSIEXEC.EXE /p "%~dp0{}" {}'.format(f, a or '/qn /norestart')
    else:
        c = 'START /W "" "%~dp0{}" {}'.format(f, a)

    cl.append((c, t.strip() or kbno(f) or f))

for i in range(0, len(cl)):
    c, t = cl[i]
    bat('<NUL (SET /P =[{:02}%%] {}...)'.format(min(((i + 1) * 100)//len(cl), 99), t))
    bat(c)
    bat('CALL :CHECKERROR')


# print bat
if args.with_reboot:
    bat('ECHO:')
    bat('ECHO Finished. The computer will be rebooted now.')
    bat('TIMEOUT /T 15')
    bat('SHUTDOWN /R /T 0')

bat('EXIT /B')
bat('')
bat(':CHECKERROR')
bat('if %ERRORLEVEL%==0 (')
bat('    ECHO: ok')
bat('    EXIT /B')
bat(')')
bat('if %ERRORLEVEL%==3010 (')
bat('    ECHO: ok, reboot required')
bat('    EXIT /B')
bat(')')
bat('if %ERRORLEVEL%==2359302 (')
bat('    ECHO: already installed')
bat('    EXIT /B')
bat(')')
bat('if %ERRORLEVEL%==-2145124329 (')
bat('    ECHO: not applicable')
bat('    EXIT /B')
bat(')')
bat('ECHO: FAILED! (%ERRORLEVEL%)')
bat('PAUSE')
bat('EXIT /B')
bat('')

