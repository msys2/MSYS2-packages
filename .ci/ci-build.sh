#!/bin/bash

set -eo pipefail

# AppVeyor and Drone Continuous Integration for MSYS2
# Author: Renato Silva <br.renatosilva@gmail.com>
# Author: Qian Hong <fracting@gmail.com>

DIR="$( cd "$( dirname "$0" )" && pwd )"

# Configure
mkdir artifacts
git remote add upstream 'https://github.com/MSYS2/MSYS2-packages'
git fetch --quiet upstream
# reduce time required to install packages by disabling pacman's disk space checking
sed -i 's/^CheckSpace/#CheckSpace/g' /etc/pacman.conf

pacman --noconfirm -Fy

# Enable colors
normal=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
cyan=$(tput setaf 6)

# Basic status function
_status() {
    local type="${1}"
    local status="${package:+${package}: }${2}"
    local items=("${@:3}")
    case "${type}" in
        failure) local -n nameref_color='red';   title='[MSYS2 CI] FAILURE:' ;;
        success) local -n nameref_color='green'; title='[MSYS2 CI] SUCCESS:' ;;
        message) local -n nameref_color='cyan';  title='[MSYS2 CI]'
    esac
    printf "\n${nameref_color}${title}${normal} ${status}\n\n"
    printf "${items:+\t%s\n}" "${items:+${items[@]}}"
}

# Run command with status
execute(){
    local status="${1}"
    local command="${2}"
    local arguments=("${@:3}")
    cd "${package:-.}"
    message "${status}"
    if [[ "${command}" != *:* ]]
        then ${command} ${arguments[@]}
        else ${command%%:*} | ${command#*:} ${arguments[@]}
    fi || failure "${status} failed"
    cd - > /dev/null
}

# Get changed packages in correct build order
list_packages() {
    # readarray doesn't work with a plain pipe
    readarray -t packages < <("$DIR/ci-get-build-order.py")
}

pkgver_from_metadata() {
    local version
    version="$(sed -n 's/^Version[[:space:]]*:[[:space:]]*//p')"
    version="${version%-*}"
    printf '%s\n' "${version#*~}" | sed 's/\./\\./g'
}

# Status functions
failure() { local status="${1}"; local items=("${@:2}"); _status failure "${status}." "${items[@]}"; exit 1; }
success() { local status="${1}"; local items=("${@:2}"); _status success "${status}." "${items[@]}"; exit 0; }
message() { local status="${1}"; local items=("${@:2}"); _status message "${status}"  "${items[@]}"; }

# Detect
list_packages || failure 'Could not detect changed files'
message 'Processing changes'

if [[ -n ${MSYS2_ARM64_CROSS:-} ]]; then
    _target_arch="aarch64"
else
    _target_arch=$(uname -m)
fi
declare -a skipped_packages=()
for package in "${packages[@]}"; do
    cd "${package}"
    readarray -d $'\0' -t arch < <(\
        set +eo pipefail
        arch=('i686' 'x86_64' 'aarch64')
        . PKGBUILD
        [[ "${#arch[@]}" -gt 0 ]] && printf "%s\0" "${arch[@]}"
    )
    if [[ ! " ${arch[*]} " =~ " ${_target_arch,,} " ]]; then
        skipped_packages+=("${package}")
    fi
    cd - > /dev/null
    unset package
done
unset _target_arch

for package in "${skipped_packages[@]}"; do
    for i in "${!packages[@]}"; do
        if [[ ${packages[i]} == $package ]]; then
            unset packages[i]
        fi
    done
    unset package
done

[ "${#packages[@]}" -eq 0 ] && success 'No changes in package recipes'

# Build
message 'Building packages' "${packages[@]}"

message 'Adding an empty local repository'
repo-add $PWD/artifacts/ci.db.tar.gz
sed -i '1s|^|[ci]\nServer = file://'"$PWD"'/artifacts/\nSigLevel = Never\n|' /etc/pacman.conf
pacman -Sy

# Remove git and python
pacman -R --recursive --unneeded --noconfirm --noprogressbar git python

# Enable linting
export MAKEPKG_LINT_PKGBUILD=1

# cross-building configuration
declare -a _makepkg_extra_args
if [[ -n ${MSYS2_ARM64_CROSS:-} ]]; then
    # The packages for the cross-building toolchain need to be installed before reaching here.
    # Use separate root when installing cross-packages to avoid conflicts with the root of the build host.
    # Set environment variable PACMAN that is also used by makepkg
    export PACMAN="pacman --root /opt/aarch64 --cachedir /opt/aarch64/var/cache/pacman/pkg --arch aarch64"
    # Use cross-building configuration for makepkg
    _makepkg_extra_args=("--config /etc/makepkg_aarch64_cross.conf")
    # Initialize pacman configuration at separate root
    mkdir -p /opt/aarch64/var/cache/pacman/pkg
    mkdir -p /opt/aarch64/var/lib/pacman
    # FIXME: We would only need the database of the MSYS2 repository (not the MinGW repositories)
    cp /etc/pacman.conf /opt/aarch64/pacman.conf
    # Load package database for separate root
    ${PACMAN} -Sy
else
    export PACMAN="pacman"
fi

message 'Building packages'
for package in "${packages[@]}"; do
    echo "::group::[build] ${package}"
    execute 'Clear cache' ${PACMAN} -Scc --noconfirm
    execute 'Fetch keys' "$DIR/fetch-validpgpkeys.sh"
    cp -r ${package} B && cd B
    message 'Building binary'
    makepkg --noconfirm --noprogressbar --nocheck --syncdeps --rmdeps --cleanbuild "${_makepkg_extra_args[@]}" || failure "${status} failed"
    cd - > /dev/null
    repo-add $PWD/artifacts/ci.db.tar.gz $PWD/B/*.pkg.tar.*
    ${PACMAN} -Sy
    cp $PWD/B/*.pkg.tar.* $PWD/artifacts
    echo "::endgroup::"

    cd B
    for pkg in *.pkg.tar.*; do
        pkgname="$(echo "$pkg" | rev | cut -d- -f4- | rev)"
        echo "::group::[install] ${pkgname}"
        grep -qFx "${package}" "$DIR/ci-dont-install-list.txt" || ${PACMAN} --noprogressbar --upgrade --noconfirm $pkg
        echo "::endgroup::"

        echo "::group::[meta-diff] ${pkgname}"
        message "Package info diff for ${pkgname}"
        diff -Nur <(${PACMAN} -Si ${MSYSTEM,,}/"${pkgname}" | sed '/Download Size/d') <(${PACMAN} -Qip "${pkg}" | sed '/Compressed Size/d') || true
        echo "::endgroup::"

        echo "::group::[file-diff] ${pkgname}"
        message "File listing diff for ${pkgname}"
        old_pkgver="$(${PACMAN} -Si ${MSYSTEM,,}/"${pkgname}" | pkgver_from_metadata || true)"
        new_pkgver="$(${PACMAN} -Qip "${pkg}" | pkgver_from_metadata || true)"
        diff -Nur \
            <(${PACMAN} -Fl ${MSYSTEM,,}/"${pkgname}" | sed -e 's|^[^ ]* |/|' -e "s|${old_pkgver}|PKGVER|g" | sort) \
            <(${PACMAN} -Ql "${pkgname}" | sed -e 's|^[^/]*||' -e "s|${new_pkgver}|PKGVER|g" | sort) || true
        echo "::endgroup::"

        echo "::group::[runtime-dependencies] ${pkgname}"
        message "Runtime dependencies for ${pkgname}"
        declare -a binaries=($(${PACMAN} -Ql $pkgname | sed -e 's|^[^ ]* ||' | grep -E ${MINGW_PREFIX}/.+\.\(dll\|exe\|pyd\)$))
        if [ "${#binaries[@]}" -ne 0 ]; then
            for binary in ${binaries[@]}; do
                echo "${binary}:"
                ldd ${binary} | GREP_COLOR="1;35" grep --color=always "msys-.*\|" \
                    || echo "        None"
            done
            message "DLL bases for ${pkgname}"
            rebase -i "${binaries[@]}" | GREP_COLOR="1;35" grep --color=always "msys-.*\|" \
                || echo "        None"
        fi
        echo "::endgroup::"

        echo "::group::[uninstall] ${pkgname}"
        message "Uninstalling $pkgname"
        grep -qFx "${package}" "$DIR/ci-dont-install-list.txt" || ${PACMAN} -R --recursive --unneeded --noconfirm --noprogressbar "$pkgname"
        echo "::endgroup::"
    done
    cd - > /dev/null

    rm -rf B
    unset package
done
success 'All packages built successfully'

cd artifacts
execute 'SHA-256 checksums' sha256sum *
