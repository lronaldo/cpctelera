# Copyright 1999-2004 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /home/cvs/uade-cvsroot/xdms/gentoo/xdms-1.3.1.ebuild,v 1.1 2004/09/11 14:46:43 shd Exp $

inherit eutils

DESCRIPTION="xDMS - Amiga DMS disk image decompressor"
HOMEPAGE="http://freshmeat.net/projects/xdms"
SRC_URI="http://ee.tut.fi/~heikki/xdms/${P}.tar.gz"

LICENSE="public-domain"
SLOT="0"
KEYWORDS="~amd64 ~x86 ~ppc"

DEPEND="virtual/libc"

src_compile() {
	cd ${S}
	./configure --prefix=/usr --package-prefix="${D}" \
		|| die "configure failed"
	emake || die "emake failed"
}

src_install() {
	make install || die "make install failed"
	dodoc COPYING xdms.txt ChangeLog.txt
}
