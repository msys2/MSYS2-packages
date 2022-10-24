# Maintainer: David Macek <david.macek.0@gmail.com>

pkgname=msys2-keyring
epoch=1
pkgver=20221024
pkgrel=1
pkgdesc='MSYS2 PGP keyring'
arch=('any')
url='https://github.com/msys2/MSYS2-keyring'
license=('GPL')
install="${pkgname}.install"
source=("https://github.com/msys2/MSYS2-keyring/archive/${pkgver}.tar.gz")
sha256sums=('b7e39eba12dd23405f0c449de765620d5158c09ce5549e23165e3c4185f138ab')

package() {
  cd "MSYS2-keyring-${pkgver}"
  make PREFIX=/usr DESTDIR=${pkgdir} install
}
