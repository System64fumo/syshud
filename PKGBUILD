pkgname=syshud-git
_pkgname=syshud
pkgver=r113.9744165
pkgrel=1
pkgdesc="Simple system status indicator"
arch=('x86_64' 'aarch64')
url="https://github.com/System64fumo/syshud"
license=('GPL3')
depends=('gtkmm-4.0' 'gtk4-layer-shell' 'wireplumber' 'libevdev')
makedepends=('git')
provides=('syshud')
conflicts=('syshud')
source=("git+$url.git")
sha256sums=('SKIP')

pkgver() {
	cd "$srcdir/$_pkgname"
	if git describe --long --tags --match 'v*' >/dev/null 2>&1; then
		git describe --long --tags --match 'v*' \
			| sed 's/^v//;s/-/./g'
	else
		printf "r%s.%s" \
			"$(git rev-list --count HEAD)" \
			"$(git rev-parse --short HEAD)"
	fi
}

build() {
	cd "$srcdir/$_pkgname"
	make
}

package() {
	cd "$srcdir/$_pkgname"
	make PREFIX=/usr DESTDIR="$pkgdir" install
}
