# fix-msys-libtool.sh — restore MSYS awareness to a libtool-generated `configure`.
#
# Usage (from a PKGBUILD prepare(), AFTER autoreconf/libtoolize/autogen):
#     source "$(dirname "${BASH_SOURCE[0]}")/fix-msys-libtool.sh"
#     fix_msys_libtool configure [ltmain.sh ...]
#
# MSYS2's libtool 2.6.2-1 keeps the `msys-` DLL-prefix soname_spec value but
# loses the `case $host_os` labels that select the Windows code paths at all
# (`grep -cE '^\s*cygwin\* \| msys\*' libtool.m4`: 2.5.4 -> 4, 2.6.2 -> 0).
# Our $host_os is plain `msys`, which matches none of the remaining labels, so
# every Windows branch is skipped: dynamic_linker=no, can_build_shared=no, and
# the build goes static-only despite --enable-shared --disable-static.
#
# The linker is not at fault -- the same configure run reports that it does
# support shared libraries. This is purely libtool's host recognition.
#
# Setting lt_cv_deplibs_check_method / lt_cv_file_magic_cmd does NOT fix it: the
# override is honoured and shared libraries are still refused, because
# can_build_shared is gated on $dynamic_linker, which lives in a different case
# block with no cache variable to override. Patching the labels is the only
# route that reaches all of them.
#
# Runs on the generated `configure` rather than m4/libtool.m4, so it works
# whether a recipe uses autoreconf, libtoolize or autogen.sh. Only the labels
# 2.5.4 actually extends are touched, so the result matches a known-good
# libtool; the dlopen and path-conversion `cygwin*)` cases are left alone
# because 2.5.4 leaves them cygwin-only too.
#
# Applies to the seed's libtool consumers (gmp, libiconv, gettext). ncurses,
# readline, bash and coreutils do not use libtool.

fix_msys_libtool() {
  local f n_composite n_labelled n_host n_comma
  for f in "$@"; do
    [ -f "$f" ] || { echo "fix_msys_libtool: no such file: $f" >&2; return 1; }

    # 1) Composite Windows labels: add msys* alongside cygwin*. Both shapes
    #    occur (with and without pw32*) and 2.5.4 makes both msys-aware, so
    #    match the family rather than one exact spelling.
    sed -i -E 's/^([[:space:]]*)cygwin\* \| (mingw\* \| windows\*)/\1cygwin* | msys* | \2/' "$f"

    # 2) The two bare `cygwin*)` cases 2.5.4 spells `cygwin* | msys*)`: the
    #    deplibs check and the DLL-prefix soname_spec. Keyed off the following
    #    line so the cygwin-only cases are left untouched.
    sed -i -E '/^[[:space:]]*cygwin\*\)$/{
      N
      /func_win32_libid is a shell function|DLLs use .* prefix rather than/s/^([[:space:]]*)cygwin\*\)/\1cygwin* | msys*)/
    }' "$f"

    # Anchor the composite count on a leading `cygwin*` so the $host-form labels
    # counted below are not tallied twice.
    n_composite=$(grep -cE '^[[:space:]]*cygwin\* \| msys\* \| mingw\*' "$f" || true)
    n_labelled=$(grep -cE '^[[:space:]]*cygwin\* \| msys\*\)$' "$f" || true)

    # 3) $host-form labels (`*-*-cygwin* | *-*-mingw* ...`), a different
    #    namespace from the $host_os labels above and also lost in 2.6.2. The one
    #    that matters is ltmain.sh's "-lc" skip list: with *-*-msys* missing,
    #    libtool appends -lc, finds no shared libc (there is none by design
    #    here), and since gmp links -no-undefined it downgrades to static.
    #
    #    This is why fixing the $host_os labels alone is not enough -- configure
    #    reports "Shared libraries: yes" and make still yields a static libgmp
    #    with dlname='' in libgmp.la.
    #
    #    Skipped when *-*-msys already appears, so a package's own msys-aware
    #    labels are never double-patched (e.g. gmp's configure.ac).
    sed -i -E '/\*-\*-msys/!s/\*-\*-cygwin\* \| (\*-\*-mingw\*)/*-*-cygwin* | *-*-msys* | \1/g' "$f"

    # 4) The comma-form label `$host,$output,$installed,$module,$dlname`, which
    #    decides where a DLL is installed. Rules 1-3 do not match its shape.
    #    On PE targets the DLL belongs in bindir next to the exes that load it,
    #    while the import lib stays in libdir; with *msys* missing tdlname stays
    #    $dlname and libtool installs the DLL into libdir instead.
    #
    #    The library itself is fine here -- this is purely an install
    #    destination bug, easy to misread as "no shared libraries were built".
    #    Native x86_64 MSYS2's libtool 2.5.4 carries this exact label and keeps
    #    every msys-*.dll in /usr/bin, so restoring it is not a novel setup.
    sed -i -E '/\*msys\*,\*lai/!s/(\*cygwin\*,\*lai,yes,no,\*\.dll) \|/\1 | *msys*,*lai,yes,no,*.dll |/' "$f"
    n_comma=$(grep -cE '\*msys\*,\*lai,yes,no,\*\.dll' "$f" || true)

    n_host=$(grep -cE '\*-\*-msys\*' "$f" || true)
    echo "fix_msys_libtool: $f -> $n_composite composite + $n_labelled labelled + $n_host host + $n_comma comma msys labels restored"

    # A silent no-op here reappears much later as a static-only build, so say so
    # loudly. 2.6.2 yields 4 composite + 3 labelled; a libtool that already
    # handles msys yields 0 and is fine.
    if [ "$n_composite" -eq 0 ] && [ "$n_labelled" -eq 0 ] && [ "$n_host" -eq 0 ] && [ "$n_comma" -eq 0 ]; then
      echo "warning: fix_msys_libtool: no msys labels in $f -- libtool may already be MSYS-aware, or the label format changed" >&2
    fi

    bash -n "$f" || { echo "error: fix_msys_libtool: $f is no longer valid shell after patching" >&2; return 1; }
  done
}
