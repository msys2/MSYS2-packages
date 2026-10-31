# #!/bin/bash
set -e  # stop if any command fails

ROOT_DIR=$(pwd)

echo "===== STEP 1: Building NEWLIB ====="
cd "$ROOT_DIR/newlib-pkgbuild"


rm -rf src pkg
makepkg -o


cd src/msys2-runtime/newlib

find . -name "Makefile.in" -delete
find . -name "Makefile" -delete

autoreconf -fvi

cd ../../..
cp -rf  src/msys2-runtime/newlib/libc/include/getopt.h .
cp -rf src/msys2-runtime/winsup/cygwin/include/* src/msys2-runtime/newlib/libc/include/ 
cp -rf ./getopt.h src/msys2-runtime/newlib/libc/include/
makepkg -e

echo "===== NEWLIB BUILD DONE ====="


# Stub the compiler's bits/c++config.h (needed to compile winsup). Resolve the
# gcc private dir from the compiler -- do NOT hardcode "15"; the installed dir
# may be "15.0.1", and the winsup compile's -isystem (derived the same way)
# would not see a stub written to the wrong version dir.
GCC_BASE="$(dirname "$(aarch64-pc-cygwin-gcc -print-libgcc-file-name)")"
mkdir -p "${GCC_BASE}/include/c++/bits"
cat > "${GCC_BASE}/include/c++/bits/c++config.h" << 'CXXCONFIG'
#ifndef _GLIBCXX_CXX_CONFIG_H
#define _GLIBCXX_CXX_CONFIG_H 1


#define __GLIBCXX__ 1


// Exception / ABI macros
#define _GLIBCXX_NOTHROW noexcept
#define _GLIBCXX_USE_NOEXCEPT noexcept
#define _GLIBCXX_THROW(...) noexcept
#define _GLIBCXX_TXN_SAFE
#define _GLIBCXX_TXN_SAFE_DYN
#define _GLIBCXX_NODISCARD [[nodiscard]]


// Visibility
#define _GLIBCXX_VISIBILITY(V) __attribute__((__visibility__(#V)))
#define _GLIBCXX_BEGIN_NAMESPACE_VERSION
#define _GLIBCXX_END_NAMESPACE_VERSION


// Feature flags
#define _GLIBCXX_USE_C99_STDLIB 1
#define _GLIBCXX_USE_C99_MATH 1
#define _GLIBCXX_USE_WCHAR_T 1
#define _GLIBCXX_HAS_GTHREADS 1



namespace std {
typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
}

// version-gated macros used by the GCC 17 libstdc++ headers
#ifndef _GLIBCXX_CONSTEXPR
#  define _GLIBCXX_CONSTEXPR constexpr
#endif
#ifndef _GLIBCXX_USE_CONSTEXPR
#  define _GLIBCXX_USE_CONSTEXPR constexpr
#endif
#ifndef _GLIBCXX14_CONSTEXPR
#  define _GLIBCXX14_CONSTEXPR constexpr
#endif
#ifndef _GLIBCXX17_CONSTEXPR
#  define _GLIBCXX17_CONSTEXPR constexpr
#endif
#ifndef _GLIBCXX20_CONSTEXPR
#  define _GLIBCXX20_CONSTEXPR constexpr
#endif
#ifndef _GLIBCXX23_CONSTEXPR
#  define _GLIBCXX23_CONSTEXPR constexpr
#endif
#ifndef _GLIBCXX26_CONSTEXPR
#  define _GLIBCXX26_CONSTEXPR constexpr
#endif
#ifndef _GLIBCXX_NOEXCEPT
#  define _GLIBCXX_NOEXCEPT noexcept
#endif
#ifndef _GLIBCXX_NOEXCEPT_IF
#  define _GLIBCXX_NOEXCEPT_IF(X) noexcept(X)
#endif
#ifndef _GLIBCXX_PURE
#  define _GLIBCXX_PURE
#endif
#ifndef _GLIBCXX_CONST
#  define _GLIBCXX_CONST
#endif
#ifndef _GLIBCXX_DEPRECATED
#  define _GLIBCXX_DEPRECATED
#endif
#ifndef _GLIBCXX_ABI_TAG_CXX11
#  define _GLIBCXX_ABI_TAG_CXX11
#endif
#endif
CXXCONFIG


echo "===== STEP 2: Building WINSUP (msys2-runtime) ====="
cd "$ROOT_DIR"

# Clean + build
cp -rf newlib-pkgbuild/msys2-runtime/ .
cp -rf newlib-pkgbuild/pkg/ .
cp -rf newlib-pkgbuild/src/ .

rm -rf src/runtime-build
cp -rf newlib-pkgbuild/src/runtime-build/aarch64-pc-cygwin/newlib  src/.

makepkg -e

echo "===== ALL BUILDS COMPLETED SUCCESSFULLY ====="