#!/usr/bin/env python3

from argparse import ArgumentParser, FileType
import re

ap = ArgumentParser()
ap.add_argument('-o', '--output', metavar='OUTFILE.BAT', type=FileType('wb'), default='-')
ap.add_argument('inlst', type=FileType('r'))

args = ap.parse_args()

def bat(line):
    args.output.write(line.encode('utf-8'))
    args.output.write(b'\r\n')

def kbno(s):
    m = re.search('[kK][bB]([0-9]+)', s)
    if m is not None:
        return 'KB{}'.format(m.group(1))
    else:
        return None

# print bat header
bat('@ECHO OFF')
bat('')
bat('if "%~1"=="" (')
bat('    ECHO Need path to mounted WIM file as first argument')
bat('    EXIT /B')
bat(')')
bat('')

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
    f, t = (l + '\t\t\t').split('\t', 1)

    c = 'DISM.EXE /Image:"%1" /Add-Package /PackagePath:"%~dp0{}" /quiet'.format(f)

    cl.append((c, t.strip() or kbno(f) or f))

for i in range(0, len(cl)):
    c, t = cl[i]
    bat('<NUL (SET /P =[{:02}%%] Integrating {}...)'.format(((i + 1) * 100)//len(cl), t))
    bat(c)
    bat('CALL :CHECKERROR')


# print bat footer
bat('ECHO:')
bat('ECHO Finished.')
bat('PAUSE')
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
