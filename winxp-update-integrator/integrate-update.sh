#!/bin/sh

set -eu
cd "$(dirname "$(readlink -f "$0")")"

is_in_list() (
    needle="$1"
    shift
    for i; do
        if [ "$i" = "$needle" ]; then
            return 0
        fi
    done
    return 1
)

# usage: integrate-update.sh kbno

kbno="$1"
branch="."
inf="update/update.inf"
copyexe=no

stringsec=""
addregsec=""
delregsec=""
crlf="$(printf '\r')
"

mkdir -p iso/I386/svcpack

if [ -d "updates-extracted/$kbno/sp3gdr" ]; then
    branch="$branch sp3gdr"
elif [ -d "updates-extracted/$kbno/sp3qfe" ]; then
    branch="$branch sp3qfe"
fi
if [ -d "updates-extracted/$kbno/wm8" ]; then
    branch="$branch wm8"
fi
if [ -d "updates-extracted/$kbno/wm9" ]; then
    branch="$branch wm9"
fi


if [ -e "updates-extracted/$kbno/update/update_sp3gdr.inf" ]; then
    inf="update/update_sp3gdr.inf"
elif [ -e "updates-extracted/$kbno/update/update_sp2gdr.inf" ]; then
    inf="update/update_sp2gdr.inf"
elif [ -e "updates-extracted/$kbno/update/update_sp3qfe.inf" ]; then
    inf="update/update_sp3qfe.inf"
elif [ -e "updates-extracted/$kbno/update/update_sp2qfe.inf" ]; then
    inf="update/update_sp2qfe.inf"
fi

printf '%s: branches %s\n' "$kbno" "$branch"

if [ -d "updates-extracted/$kbno/asms" ]; then
    printf '%s: found asms --> copying exe\n' "$kbno"
    copyexe=yes
fi

foundsome=no
for b in $branch; do
    for n in $(find updates-extracted/$kbno/$b -maxdepth 1 -type f -printf '%f\n'); do
        found=no

        if is_in_list "$n" spmsg.dll spuninst.exe spupdsvc.exe empty.cat; then
            continue
        fi

        foundsome=yes

        if [ "$n" = "ftpsvc2.dll" ]; then
            f="ftpsv251.dll"
        elif [ "$n" = "uiautomationcore.dll" ]; then
            f="uia_core.dll"
        else
            f="$n"
        fi

        basename="${f%.*}"
        extension="${f##*.}"
        cabname="$(printf '%s.%.2s_\n' "$basename" "$extension")"

        # directly integrate selected new files
        if is_in_list "$f" iacenc.dll xpsp4res.dll uia_core.dll; then
            if [ ! -e "iso/I386/$cabname" ]; then
                rm -rf tmp/cab
                mkdir -p tmp/cab
                touch -d'1999-09-09T19:09:09' "tmp/cab/$f"
                gcab -cnz "iso/I386/$cabname" tmp/cab/*

                if [ "$f" != "$n" ]; then
                    r=",$n"
                else
                    r=""
                fi

                find iso/I386 -maxdepth 1 -type f -iname txtsetup.sif -exec sed -i "s/\\[SourceDisksFiles\\]/[SourceDisksFiles]\\r\\n$f = 100,,,,,,,2,0,0$r/" '{}' \;
                #find iso/I386 -maxdepth 1 -type f -iname layout.inf -exec sed -i "s/\\[SourceDisksFiles\\]/[SourceDisksFiles]\\r\\n$f = 100,,,,,,,2,0,0/" '{}' \;
                find iso/I386 -maxdepth 1 -type f -iname dosnet.inf -exec sed -i "s/\\[Files\\]/[Files]\\r\\nd1,$f/" '{}' \;
            fi
        fi

        # HACK: ensure bthport.sys is always installed, so that WU doesn't complain about kb951376
        # (Source: https://msfn.org/board/topic/119418-integrated-kb951376-yet-still-pop-up-in-windows-update/)
        if [ "$f" = "bthport.sys" ]; then
            find iso/I386 -maxdepth 1 -type f -iname txtsetup.sif -exec sed -i 's/bthport\.sys *= *100,,,,,,,4,1,3/bthport.sys = 100,,,,,,,4,0,0/' '{}' \;
        fi

        for r in $(find iso/I386 -maxdepth 1 -type f -iname "$f" -printf '%P'); do
            printf '%s: FOUND %s/%s--COPY-->%s\n' "$kbno" "$b" "$n" "$r"
            found=yes
            find "updates-extracted/$kbno/$b/$f" -newer "iso/I386/$r" -exec cp -a '{}' "iso/I386/$r" \;
        done

        for c in $(find iso/I386 -maxdepth 1 -type f -iname "$cabname" -printf '%P'); do
            printf '%s: FOUND %s/%s--CAB-->%s\n' "$kbno" "$b" "$n" "$c"
            found=yes

            rm -rf tmp/cab
            mkdir -p tmp/cab
            cabextract -qsd tmp/cab "iso/I386/$c"
            find tmp/cab \( -iname "${f##*/}" -a \! -newer "updates-extracted/$kbno/$b/$n" \) -exec cp -a "updates-extracted/$kbno/$b/$n" \{\} \;
            gcab -cnz "iso/I386/$c" tmp/cab/*
        done

        for d in $(find tmp -maxdepth 1 -type d -iname '*.cab' -printf '%f\n'); do
            for r in $(find "tmp/$d" -maxdepth 1 -type f -iname "$f" -printf '%P\n'); do
                printf '%s: FOUND %s/%s--%s-->%s\n' "$kbno" "$b" "$n" "$d" "$r"
                found=yes
                find "updates-extracted/$kbno/$b/$n" -newer "tmp/$d/$r" -exec cp -a '{}' "tmp/$d/$r" \;
            done
        done

        if [ "$found" != yes ]; then
            printf '%s: MISSING %s/%s --> copying exe\n' "$kbno" "$b" "$f"
            copyexe=yes
        fi
    done
done

# copy catalog files
find updates-extracted/$kbno/update -iname '*.cat' -exec cp -a -t iso/I386/svcpack '{}' \;

# HACK: check reg additions and add special inf if needed
regfound=no
while IFS='	' read -r s l; do
    if [ "$s" = "Save.Reg.For.Uninstall" ] || [ "$s" = "mscoree.Add.Reg.Session" ]; then
        : # don't need to consider these
    elif is_in_list "$s" "Product.Add.Reg" "General.Add.Reg" "OleAcc.Add.Reg" "win32k.Add.Reg.Session" "UIACore.Add.Reg"; then
        # skip ARP entries, they are incomplete anyway
        if printf '%s\n' "$l" | egrep -ivq '^HKLM,SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\' &&
            printf '%s\n' "$l" | egrep -ivq '^HKLM,SOFTWARE\\Microsoft\\Updates\\Windows XP\\' &&
            printf '%s\n' "$l" | egrep -ivq '^HKLM,%UpdateRegKey%\\' &&
            printf '%s\n' "$l" | egrep -ivq '^HKLM,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup", *"LogLevel"' &&
            printf '%s\n' "$l" | egrep -ivq '^HKLM,"?SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\' &&
            printf '%s\n' "$l" | egrep -ivq '"rundll32\.exe apphelp\.dll,ShimFlushCache"\s*$'; then
            addregsec="$addregsec$l$crlf"
        fi
    elif [ "$s" = "Product.Del.Reg" ]; then
        if ! printf '%s\n' "$l" | egrep -q '^HKLM,"?SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\'; then
            delregsec="$delregsec$l$crlf"
        fi
    elif [ "$s" = "Strings" ]; then
        stringsec="$stringsec$l$crlf"
    elif printf '%s\n' "$l" | egrep -q '^\s*HKLM,'; then
        if printf '%s\n' "$l" | egrep -ivq '^HKLM,SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\' &&
            printf '%s\n' "$l" | egrep -ivq '^HKLM,SOFTWARE\\Microsoft\\Updates\\Windows XP\\' &&
            printf '%s\n' "$l" | egrep -ivq '^HKLM,%UpdateRegKey%\\' &&
            printf '%s\n' "$l" | egrep -ivq '^HKLM,"?SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup"?, *"LogLevel"' &&
            printf '%s\n' "$l" | egrep -ivq '^HKLM,"?SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\' &&
            printf '%s\n' "$l" | egrep -ivq '"rundll32\.exe apphelp\.dll,ShimFlushCache"\s*$'; then
            # these might be missing reg changes
            printf '%s: POSSIBLE REG ENTRY [%s] %s\n' "$kbno" "$s" "$l" 1>&2
            regfound=yes
        fi
    fi

    if [ "$s" = "Configuration" ] && [ "${l#CustomizationDll=}" != "$l" ]; then
        # skip well known harmless ones
        if [ "$l" != "CustomizationDll=mpsyschk.dll" ]; then
            printf '%s: Update Customization DLL %s --> copying exe\n' "$kbno" "$l"
            regfound=yes
        fi
    fi
done << EOF
$(./readini.sh <"updates-extracted/$kbno/$inf")
EOF

if [ "$regfound" = yes ]; then
    printf '%s: found reg customization --> copying exe\n' "$kbno"
    copyexe=yes
fi

if [ "$foundsome" != yes ]; then
    printf '%s: found no files --> copying exe\n' "$kbno"
    copyexe=yes
fi

if [ "$copyexe" = yes ]; then
    find update-repo -iname "*$kbno*.exe" -exec cp -a '{}' "iso/I386/svcpack/$kbno.exe" \;
elif [ -n "$addregsec" ] || [ -n "$delregsec" ]; then
    # write INF file to apply reg changes
    (
        printf '[Version]\r\n'
        printf 'Signature="$Windows NT$"\r\n'
        printf '\r\n'
        printf '[DefaultInstall]\r\n'
        printf 'AddReg=HotfixAddReg\r\n'
        printf 'DelReg=HotfixDelReg\r\n'
        printf '\r\n'
        printf '[HotfixAddReg]\r\n'
        printf '%s\r\n' "$addregsec"
        printf '[HotfixDelReg]\r\n'
        printf '%s\r\n' "$delregsec"
        if [ -n "$(printf '%s\n' "$addregsec" | grep -o '%')$(printf '%s\n' "$delregsec" | grep -o '%')" ]; then
            printf '[Strings]\r\n'
            printf '%s\r\n' "$stringsec"
        fi
        printf '; created by hotfix integrator\r\n'
    ) > iso/I386/svcpack/$kbno.inf
    #printf 'rundll32.exe advpack.dll,LaunchINFSectionEx %%~dp0%s.inf,DefaultInstall,,4,N\r\n' "$kbno" > iso/I386/svcpack/$kbno.cmd
fi
