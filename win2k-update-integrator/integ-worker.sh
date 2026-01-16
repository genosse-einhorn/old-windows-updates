#!/bin/sh
set -eu

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

is_file_newer() (
    candidate="$1"
    existing="$2"

    cver="$(exiftool -p '$FileVersion' "$candidate" 2>/dev/null | xargs)"
    ever="$(exiftool -p '$FileVersion' "$existing" 2>/dev/null | xargs)"

    if [ -n "$cver" ] && [ -n "$ever" ]; then
        # have version numbers, compare them
        highestver="$(printf '%s\n%s\n' "$cver" "$ever" | sort -Vr | head -n1)"
        if [ "$highestver" = "$cver" ]; then
            return 0
        else
            return 1
        fi
    else
        # no version numbers, use dates
        if [ -n "$(find "$candidate" -newer "$existing" || true)" ]; then
            return 0
        else
            return 1
        fi
    fi
)

if [ -d "$1" ]; then
    # launched with dir
    printf 'Integrating %s\n' "$1"

    dir="$1"
    kb="${1##*/}"
    file="$kb"
else
    # launched with update exe
    kb="$1"
    file="$2"
    dir="iso/i386/svcpack/$kb"

    printf 'Integrating %s\n' "$file"
    wine updates/$file /x:$(winepath -w "$dir") /quiet

    # fixup filename casing
    find "$dir" -depth -printf '%f:%h\n' | while IFS=':' read -r name dir; do mv -n -T "$dir/$name" "$dir/$(printf '%s\n' "$name" | tr 'A-Z' 'a-z')"; done

    # replace update.exe
    find updater-files -mindepth 1 -type f -printf '%P\n' | while read -r f; do
        ln -f "updater-files/$f" "$dir/$f"
    done
fi

foundall=yes
foundsome=no
for filename in $(find "$dir" -type f -printf '%P\n'); do
    if is_in_list "$filename" spmsg.dll spuninst.exe spupdsvc.exe sprecovr.exe iecustom.dll updcustom.dll || [ "${filename%.cat}" != "$filename" ] || [ "${filename#update/}" != "$filename" ] || [ "${filename#xpsp2_binarydrop/}" != "$filename" ] || [ "${filename#wm9/}" != "$filename" ]; then
        continue
    fi

    t="$filename"
    if [ "${file#mdac}" != "$file" ] && [ "${filename#files/}" != "$filename" ]; then
        # MDAC update has files in subfolder
        t="${filename#files/}"
    fi
    if [ "${file#windowsmedia}" != "$file" ] || [ "${file#windows2000-windowsmedia}" != "$file" ]; then
        # media player updates are also in subfolder
        t="${t#wm41/}"
        t="${t#wm64/}"
        t="${t#generic/}"
        t="${t#generic_copyalways/}"
        t="${t#generic_nodelay/}"
    fi
    if [ "$filename" = smtpsvc.dll ]; then
        # has a prefixed name in ims.cab
        t="smtp_smtpsvc.dll"
    fi

    # HACK for selected new files
    if [ ! -e "iso/i386/$t" ] && is_in_list "$filename" verclsid.exe; then
        ln -f "$dir/$filename" "iso/i386/$t"
        sed -i "s/\\[SourceDisksFiles\\]/[SourceDisksFiles]\\r\\n$t = 2,,,,,,,2,0,0/" iso/i386/txtsetup.sif
    fi

    found=no
    tbase="${t%.*}"
    text="${t##*.}"
    tcab="$tbase.$(printf '%.2s\n' "$text")_"

    if [ -e "iso/i386/$t" ]; then
        # raw file in i386 directory -> replace
        found=yes
        if is_file_newer "$dir/$filename" "iso/i386/$t"; then
            printf 'FOUND: %s --> %s\n' "$filename" "$t"
            ln -f "$dir/$filename" "iso/i386/$t"
        else
            printf 'SKIP: %s --> %s (already newer)\n' "$filename" "$t"
        fi
    elif [ -e "iso/i386/$tcab" ]; then
        # compressed file in i386 directory -> delete, replace with raw file
        found=yes
        printf 'FOUND: %s --> %s\n' "$filename" "$tcab"
        rm -f "iso/i386/$tcab"
        ln -f "$dir/$filename" "iso/i386/$t"
    else
        # included in driver.cab/sp4.cab/... -> copy to i386 directory
        # will be picked up by setup in preference to the file inside the cab
        for cab in $(find tmp -maxdepth 1 -type d -iname '*.cab' -printf '%f\n'); do
            if [ -e "tmp/$cab/$t" ]; then
                found=yes
                if is_file_newer "$dir/$filename" "tmp/$cab/$t"; then
                    printf 'FOUND: %s --> %s/%s\n' "$filename" "$cab" "$t"
                    ln -f "$dir/$filename" "iso/i386/$t"
                else
                    printf 'SKIP: %s --> %s (already newer)\n' "$filename" "$t"
                fi
            fi
        done
    fi

    # extra HACK for disc space savings: integrate back into other updates
    for i in $(find iso/i386/svcpack -maxdepth 1 -type d -iname 'kb*' -printf '%f\n'); do
        if [ "$i" = "$kb" ]; then
            continue
        fi

        if [ -e "iso/i386/svcpack/$i/$filename" ]; then
            found=yes
            if is_file_newer "$dir/$filename" "iso/i386/svcpack/$i/$filename"; then
                printf 'FOUND: %s --> %s/%s\n' "$filename" "$i" "$filename"
                ln -f "$dir/$filename" "iso/i386/svcpack/$i/$filename"

                # patch update.ver
                (
                    prefix="$(printf '%s=\n' "$filename" | sed 's/\//\\\\/g')"
                    egrep -vi "^$prefix" iso/i386/svcpack/$i/update/update.ver || true
                    egrep -i "^$prefix" iso/i386/svcpack/$kb/update/update.ver || true
                ) > tmp/update.ver.new
                cp -a tmp/update.ver.new "iso/i386/svcpack/$i/update/update.ver"
            else
                printf 'SKIP: %s --> %s/%s (already newer)\n' "$filename" "$i" "$filename"
                printf 'FIXME! should we reverse-copy this?\n'
            fi
        fi
    done

    if [ "$found" = yes ]; then
        foundsome=yes
    else
        foundall=no
        printf 'MISSING: %s\n' "$filename"
    fi
done

# special HACK: integrate KB919521
if [ "$kb" = "kb891861" ]; then
    iconv -f utf-16le -t utf-8 iso/i386/hivesys.inf > tmp/hivesys-utf8.inf
    sed -i -E 's/^\[AddReg\]/[AddReg]\r\nHKLM,"SYSTEM\\CurrentControlSet\\Control\\HAL","14140000FFFFFFFF",0x00010001,0x00000010    ; KB919521/' tmp/hivesys-utf8.inf
    iconv -f utf-8 -t utf-16le tmp/hivesys-utf8.inf > iso/i386/hivesys.inf
fi

if [ -d "$dir/update" ]; then
    find "$dir/update" -iname '*.cat' -exec ln -t iso/i386/svcpack '{}' \;
fi

# HACK: direct integration of whitelisted updates
if [ -e direct-integration-whitelist.txt ] && is_in_list "$kb" $(cat direct-integration-whitelist.txt); then
    printf 'DIRECT INTEGRATION: whitelisted %s\n' "$kb"

    if [ "$foundsome" != yes ]; then
        printf 'WARN: directly integrating update with no files\n'
    fi
    if [ "$foundall" != yes ]; then
        printf 'WARN: directly integrating update with missing files\n'
    fi

    stringsec=""
    addregsec=""
    delregsec=""
    crlf="$(printf '\r')
"

    inf="update/update.inf"
    if [ -e "$dir/update/update_w2k.inf" ]; then
        inf="update/update_w2k.inf"
    fi
    if [ -e "$dir/update/update_win2k.inf" ]; then
        inf="update/update_win2k.inf"
    fi

    while IFS='	' read -r s l; do
        if is_in_list "$s" "Product.Add.Reg" "MSI.AddReg"; then
            # skip ARP entries, they are incomplete anyway
            if printf '%s\n' "$l" | egrep -ivq '^HKLM,"?SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\'; then
                addregsec="$addregsec$l$crlf"
            fi
        elif [ "$s" = "Product.Del.Reg" ]; then
            if ! printf '%s\n' "$l" | egrep -q '^HKLM,"?SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\'; then
                delregsec="$delregsec$l$crlf"
            fi
        elif [ "$s" = "Strings" ]; then
            stringsec="$stringsec$l$crlf"
        fi
    done <<EOF
$(./readini.sh <"$dir/$inf")
EOF

    if [ -n "$addregsec$delregsec" ]; then
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
            printf '[Strings]\r\n'
            printf '%s\r\n' "$stringsec"
            printf '; created by hotfix integrator\r\n'
        ) > iso/i386/svcpack/$kb.inf
    fi

    rm -rf "iso/i386/svcpack/$kb"
fi

