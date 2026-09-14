#!/usr/bin/env bash
# lint-arm64-pkgnames.sh — host-clobber guard for the ARM64 cross recipes.
#
# Enforces that every ARM64 stage recipe uses a collision-free `cross-msysarm64-*`
# pkgname. A recipe named `msys2-runtime` / `msys2-runtime-devel` (the HOST x86_64
# MSYS2 runtime names) lets a `pacman -U` on the build box overwrite the host
# runtime with ARM64 binaries — which would let a pacman -U overwrite the host runtime with ARM64 binaries.
#
# The pkgname is the complete guard: pacman keys install/upgrade/replace on the
# package NAME, so a differently-named package (even one that ships files under
# /usr/bin, like the cross binutils) cannot downgrade or overwrite the host
# `msys2-runtime`. The dangerous case is a recipe literally named
# `msys2-runtime`.
#
# Exit 0 = clean, 1 = violation. No build deps; runs anywhere with bash.
set -u

REPO_ROOT="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
cd "$REPO_ROOT" || { echo "lint: cannot cd to $REPO_ROOT"; exit 2; }

# Recipe dirs that build ARM64 cross artifacts. Globs; missing dirs are skipped.
mapfile -t PKGBUILDS < <(
  find cross-msysarm64-* msys2-runtime-aarch64* -name PKGBUILD 2>/dev/null | sort
)

if [ "${#PKGBUILDS[@]}" -eq 0 ]; then
  echo "lint: no ARM64 cross PKGBUILDs found under $REPO_ROOT — nothing to check"
  exit 0
fi

fail=0
note() { printf '  %s\n' "$*"; }

for pb in "${PKGBUILDS[@]}"; do
  echo "== $pb"

  # Resolve pkgname the way makepkg does: source top-level assignments in a
  # subshell (functions are only defined, never executed at source time) and
  # read the expanded ${pkgname[@]}. Fall back to grep if sourcing fails.
  names="$(
    unset pkgname _realname pkgbase
    # shellcheck disable=SC1090
    if source "$pb" >/dev/null 2>&1; then
      printf '%s\n' "${pkgname[@]}"
    fi
  )"
  if [ -z "$names" ]; then
    note "WARNING: could not source $pb; falling back to raw pkgname= line"
    names="$(sed -n 's/^pkgname=//p' "$pb" | tr -d '()' | tr ' ' '\n')"
  fi

  while IFS= read -r n; do
    [ -z "$n" ] && continue
    case "$n" in
      msys2-runtime|msys2-runtime-devel)
        note "ERROR: host-colliding pkgname '$n' (would clobber the host x86_64 MSYS2 runtime)"
        fail=1 ;;
      cross-msysarm64-*)
        note "ok pkgname '$n'" ;;
      *)
        note "ERROR: pkgname '$n' is not 'cross-msysarm64-*' (collision-free naming required)"
        fail=1 ;;
    esac
  done <<< "$names"
done

echo
if [ "$fail" -ne 0 ]; then
  echo "lint-arm64-pkgnames: FAIL — fix the pkgname/staging violations above."
  exit 1
fi
echo "lint-arm64-pkgnames: PASS — all ARM64 cross recipes use collision-free names."
exit 0
