# Maintainer: Ricky <rickleaf.wu@gmail.com>
_realname="emacs"
pkgname="emacs"
pkgver=25.1
pkgrel=1
pkgdesc="The extensible, customizable, self-documenting, real-time display editor (msys2)"
url="http://www.gnu.org/software/${_realname}/"
license=('GPL3')
arch=('any')
depends=('ncurses')
groups=('editors')
makedepends=('gawk' 'perl' 'python2' 'python3' 'ruby' 'libiconv' 'ncurses-devel' 'libcrypt-devel')
options=('strip')
source=("http://ftp.gnu.org/gnu/${_realname}/${_realname}-${pkgver}.tar.xz"{,.sig}
        'configure.ac.diff')
sha256sums=('19f2798ee3bc26c95dca3303e7ab141e7ad65d6ea2b6945eeba4dbea7df48f33'
            'fce9c0494bec47106e6d6853e44b825e49558a8da009ec47930d7c7d6814037a'
            '0cb204f2cab4740d27a47a4adc7e4168d6034e86cd522605a85bbd03fb1db59d')

prepare() {
  cd "${_realname}-${pkgver}"
  patch --binary --forward -p0 < "${srcdir}/configure.ac.diff"
  ./autogen.sh
}

build() {
	
  cd "${srcdir}/${_realname}-${pkgver}"

  CPPFLAGS="-DNDEBUG"
  CFLAGS="-pipe -O3 -fomit-frame-pointer -funroll-loops"
  LDFLAGS="-s -Wl,-s"
  ./configure \
    --prefix=/usr \
    --build="${CHOST}" \
    --with-x-toolkit=no \
    --with-sound=yes \
    --with-modules \
    --without-compress-install

  make
}

package() {
  cd "${srcdir}"/${_realname}-${pkgver}
  make DESTDIR="${pkgdir}" install
  rm -f "${pkgdir}/usr/bin/ctags.exe"
  rm -f "${pkgdir}/usr/share/man/man1/ctags.1.gz"
  rm -f "${pkgdir}/usr/share/info/info.info.gz"

  local dir="${pkgdir}/usr/share/${_realname}"
        dir="${dir}/$(ls -1 ${dir} | grep -E '([0-9]+\.[0-9]+)(\.[0-9]+)?')/src"

  mkdir -p "${dir}"
  cd "${srcdir}/${_realname}-${pkgver}/src"
  cp *.c *.h *.m "${dir}"
  
}

# TODO:
# Patch `shell-file-name' default in the C source code similarly to
# `source-directory'.
