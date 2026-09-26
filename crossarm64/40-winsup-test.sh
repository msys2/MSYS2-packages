#!/usr/bin/env bash
# Stage 4 - run the winsup (Cygwin) testsuite against the freshly built runtime.
#
#   bash crossarm64/40-winsup-test.sh
#
# Derived from .github/workflows/cygwin.yml ("Test (winsup testsuite)") in
# msys2-runtime, adapted for a LOCAL aarch64-pc-cygwin build:
#
#   - triple is aarch64-pc-cygwin, not aarch64-pc-msys
#   - runtime_root is testinst/usr/bin, not testinst/bin  (b3059f23e + 60cbfd3c2)
#   - no GitHub Actions annotations / artifact upload
#
# Non-gating by design: it reports, it does not fail the shell on test failures.
set -uo pipefail

RT2="${RT2:-$HOME/MSYS2-packages/msys2-runtime-aarch64-stage-2}"
TRIPLE="${TRIPLE:-aarch64-pc-cygwin}"
MINGW_GCC="${MINGW_GCC:-/opt/bin/aarch64-w64-mingw32-gcc}"
TIMEOUT="${TIMEOUT:-60m}"
JOBS="${JOBS:-$(nproc)}"

# real symlinks for the symlink/readlink/lstat tests
export MSYS="winsymlinks:sys${MSYS:+ $MSYS}"

say()  { printf '  %s\n' "$*"; }
warn() { printf '  WARNING: %s\n' "$*" >&2; }

RB="$(find "$RT2/src" -type d -name runtime-build 2>/dev/null | head -1)"
[ -n "$RB" ] || { echo "ERROR: runtime-build not found under $RT2/src - build the runtime first"; exit 1; }
TS="$RB/testsuite"
SRC="$(dirname "$(find "$RT2/src" -path '*/winsup/testsuite/cygrun.c' 2>/dev/null | head -1)")"
[ -f "$TS/Makefile" ] || { echo "ERROR: testsuite not configured ($TS/Makefile missing)"; exit 1; }

echo "=== winsup testsuite ==="
say "runtime-build : $RB"
say "testsuite     : $TS"
say "source        : $SRC"

# runtime_root: the tests and cygserver MUST load the same msys-2.0.dll.  The
# SysV-IPC namespace is keyed on installation_key, a hash of the DLL's full path
# (cygheap.cc init_installation_root -> shared.cc get_shared_parent_dir), so a
# daemon and client loading it from different directories land in different
# namespaces and msgget/semget/shmget all fail even with cygserver up.
export runtime_root="$TS/testinst/usr/bin"
export PATH="$runtime_root:$PATH"
say "runtime_root  : $runtime_root"

# busybox is used by the LTP tests
dst=/usr/libexec/busybox/bin; mkdir -p "$dst"
if [ ! -f "$dst/busybox.exe" ]; then
  if command -v busybox >/dev/null 2>&1; then
    cp -f "$(command -v busybox)" "$dst/busybox.exe"
  else
    warn "busybox not found - the LTP tests that need sh/sleep/ls will fail"
  fi
fi

# 1. -lcygwin must resolve to the msys import library
( cd "$RB/cygwin" && { [ -e libcygwin.a ] || ln -s libmsys-2.0.a libcygwin.a; } ) 2>/dev/null

# 2. cygrun.exe - a Win32-only launcher, built with the mingw cross compiler
mkdir -p "$TS/mingw"
if [ -x "$MINGW_GCC" ]; then
  "$MINGW_GCC" "$SRC/cygrun.c" -o "$TS/mingw/cygrun.exe" -lkernel32 \
    && say "cygrun.exe built" || warn "cygrun.exe build failed"
else
  warn "mingw gcc not at $MINGW_GCC - cygrun.exe not rebuilt"
fi

# 3. cygrun.sh: MSYS2 has no cygdrop, and the dll is msys-2.0.dll
sed -i -e 's|cygdrop $cygrun $exe|$cygrun $exe|' -e 's|cygwin1\.dll|msys-2.0.dll|' "$SRC/cygrun.sh"

# 4. stage the freshly built ARM64 DLL where the tests load it
mkdir -p "$runtime_root"
DLL="$(find "$RB/cygwin" -maxdepth 1 \( -name new-msys-2.0.dll -o -name msys-2.0.dll \) 2>/dev/null | head -1)"
if [ -n "$DLL" ]; then cp -f "$DLL" "$runtime_root/msys-2.0.dll"; say "staged $(basename "$DLL")"; else warn "no built DLL found in $RB/cygwin"; fi

# 4b. make sure gcc links the REAL crt0.o, not the bootstrap stub (a stub makes
#     cygserver hang at startup and the SysV-IPC tests fail)
REAL_CRT0="$RB/cygwin/crt0.o"
if [ -f "$REAL_CRT0" ] && "${TRIPLE}-nm" "$REAL_CRT0" 2>/dev/null | grep -q 'U msys_crt0'; then
  real_abs="$(cd "$(dirname "$REAL_CRT0")" && pwd)/crt0.o"
  targets="$("${TRIPLE}-gcc" -print-file-name=crt0.o) /usr/${TRIPLE}/lib/crt0.o $(find "$RT2/src" -name crt0.o 2>/dev/null)"
  for t in $targets; do
    [ -f "$t" ] || continue
    [ "$(cd "$(dirname "$t")" && pwd)/$(basename "$t")" = "$real_abs" ] && continue
    "${TRIPLE}-nm" "$t" 2>/dev/null | grep -q 'U msys_crt0' || { cp -f "$REAL_CRT0" "$t" && say "crt0-fix: replaced stub at $t"; }
  done
  gcc_crt0="$("${TRIPLE}-gcc" -print-file-name=crt0.o)"
  "${TRIPLE}-nm" "$gcc_crt0" 2>/dev/null | grep -q 'U msys_crt0' \
    && say "crt0-fix: OK, gcc links the real crt0.o" || warn "gcc-linked crt0.o still looks like a stub"
else
  warn "real crt0.o missing or not real - cygserver/tests may hang"
fi

# 5. relink cygserver against the real crt0.o
( cd "$RB/cygserver" && rm -f cygserver.exe && make OBJEXT=o ) >/dev/null 2>&1 \
  && say "cygserver relinked" || warn "cygserver relink failed"
# never leave a DLL next to cygserver.exe: Windows searches the exe dir before
# PATH, which would split the IPC namespace (see the runtime_root note above)
rm -f "$RB/cygserver/msys-2.0.dll"

# 6. start cygserver for the SysV IPC tests
if ! ps -W 2>/dev/null | grep -iq '[c]ygserver'; then
  ( "$RB/cygserver/cygserver.exe" -d -e > /tmp/cygserver.log 2>&1 & )
  sleep 2
fi
if ps -W 2>/dev/null | grep -iq '[c]ygserver'; then say "cygserver up"
else warn "cygserver not running - SysV IPC tests will fail"; cat /tmp/cygserver.log 2>/dev/null | sed 's/^/      /'; fi

# 7. devdsp wedges the GCC-15 optimizer; stub it to exit 77 (= SKIP)
[ -f "$SRC/winsup.api/devdsp.c" ] && printf 'int main(void){return 77;}\n' > "$SRC/winsup.api/devdsp.c"

# 8+9. drop cygload (unported to AArch64), drop -lwinmm, map testinst/tmp -> tmp
sed -i 's/^\(.*cygload.*\)$/# \1/'             "$TS/Makefile"
sed -i '/winsup_api_devdsp_LDADD/s/-lwinmm //' "$TS/Makefile"
# NOTE: do NOT apply CI's  sed 's#testinst/tmp#tmp#g'  here.  That is correct for
# CI's testinst/bin layout, but ours is testinst/usr/bin (b3059f23e + 60cbfd3c2).
# MSYS derives its root by stripping THREE components from the DLL path
# (msys-2.0.dll, bin, usr), so for testinst/usr/bin/msys-2.0.dll the root is
# testinst and "/tmp" resolves to $TS/testinst/tmp.  Without that directory every
# LTP test dies in tst_tmpdir():
#     tst_tmpdir(): mkdir(/tmp/dupffffcac9.0, 0777) failed; errno = 2
# which shows up as ~118 spurious FAILs.
mkdir -p "$TS/testinst/tmp" "$TS/tmp"
# defeat automake maintainer-mode regen so the edits above survive make check
touch "$SRC/Makefile.am" "$SRC/Makefile.in" "$RB/config.status" "$TS/Makefile" 2>/dev/null

# automake's parallel-tests harness gives each test its own .log/.trs and the
# LTP tests name their temp dirs after their pid, so -j is safe here.  If a
# result ever looks order-dependent, re-run with JOBS=1 to confirm.
echo "=== running make check -j$JOBS (timeout $TIMEOUT) ==="
( cd "$TS" && timeout --kill-after=30s "$TIMEOUT" make -j"$JOBS" check AM_COLOR_TESTS=always ) \
  || warn "make check returned non-zero (non-gating)"

# validity gate: sample-fail.c is `return 1` and is a listed XFAIL.  If it XPASSes,
# exit codes are not propagating and every PASS is meaningless.
SF="$(find "$TS" -name 'sample-fail.trs' 2>/dev/null | head -1)"
if [ -n "$SF" ] && grep -q '^:test-result: XPASS' "$SF"; then
  echo
  echo "  *** INVALID RUN: the sample-fail canary (return 1) XPASSed."
  echo "  *** Test exes are not returning their exit code - all PASS counts are false."
fi

echo "=== summary ==="
for t in PASS FAIL XFAIL XPASS SKIP ERROR; do
  n=$(find "$TS" -name '*.trs' -exec grep -h "^:test-result: $t$" {} \; 2>/dev/null | wc -l)
  printf '  %-6s %s\n' "$t" "$n"
done
echo "  logs: $TS  (*.log, *.trs)"
