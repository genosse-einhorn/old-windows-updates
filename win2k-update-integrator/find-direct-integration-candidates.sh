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

kblist="$(cd updates; for i in *-kb*.exe; do printf '%s\n' "$i" | sed 's/^.*-\(kb[0-9]\+\).*\.exe$/\1/'; done | sort -Vu)"

for kbno in $kblist; do
    if is_in_list "$kbno" kb842773 kb891861 kb958470; then
        # known not to work, remove logspam
        continue
    fi

    dir="tmp/upd"
    file="$(cd updates; ls *-$kbno*.exe | head -n1)"
    rm -rf "$dir"
    mkdir -p "$dir"
    wine updates/$file /x:$(winepath -w "$dir") /quiet

    inf="update/update.inf"
    if [ -e "$dir/update/update_w2k.inf" ]; then
        inf="update/update_w2k.inf"
    fi
    if [ -e "$dir/update/update_win2k.inf" ]; then
        inf="update/update_win2k.inf"
    fi

    regfound=no
    while IFS='	' read -r s l; do
        if is_in_list "$s" Save.Reg.For.Uninstall Product.Add.Reg Product.Del.Reg MSI.AddReg; then
            : # don't need to consider these
        elif printf '%s\n' "$l" | egrep -q '^\s*HKLM,'; then
            if printf '%s\n' "$l" | egrep -ivq '^HKLM,SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\' &&
                printf '%s\n' "$l" | egrep -ivq '^HKLM,"?SOFTWARE\\Microsoft\\Updates\\' &&
                printf '%s\n' "$l" | egrep -ivq '^HKLM,%UpdateRegKey%\\' &&
                printf '%s\n' "$l" | egrep -ivq '^HKLM,"?SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup"?, *"LogLevel"' &&
                printf '%s\n' "$l" | egrep -ivq '^HKLM,"?SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\' &&
                printf '%s\n' "$l" | egrep -ivq '^HKLM,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon","BufferPolicyReads",0x10001,1'; then
                # these might be missing reg changes
                printf '%s: POSSIBLE REG ENTRY [%s] %s\n' "$kbno" "$s" "$l" 1>&2
                regfound=yes
            fi
        fi

        if [ "${s#ProcessesToRun}" != "$s" ]; then
            printf '%s: [%s] %s\n' "$kbno" "$s" "$l" 1>&2
            regfound=yes
        fi

        if [ "$s" = "Configuration" ] && [ "${l#CustomizationDll=}" != "$l" ]; then
            # skip known harmless ones
            if ! is_in_list "${l#CustomizationDll=}" iecustom.dll SQLSTPCustomDll.dll; then
                printf '%s: Update Customization DLL %s\n' "$kbno" "$l" 1>&2
                regfound=yes
            fi
        fi
    done << EOF
$(./readini.sh <"$dir/$inf")
EOF

    if [ "$regfound" != yes ]; then
        printf '%s\n' "$kbno"
    fi
done
