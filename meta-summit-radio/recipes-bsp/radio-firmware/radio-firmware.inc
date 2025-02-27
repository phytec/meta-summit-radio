SECTION = "kernel"

inherit allarch

SUMMIT_URI ?= "${SUMMIT_URI_BASE}"
SUMMIT_URI:summit-internal = "${SUMMIT_URI_BASE}/firmware/${PV}"

SRC_URI = "${SUMMIT_URI}/laird-${BPN}-${PV}.tar.bz2;name=${BPN}"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

FILES:${PN} += "${nonarch_base_libdir}"

S = "${WORKDIR}"

ALLOW_EMPTY:${PN}-dev = "0"
ALLOW_EMPTY:${PN}-dbg = "0"

do_install() {
	install -d  ${D}${nonarch_base_libdir}
	cp -r ${S}/lib/* ${D}${nonarch_base_libdir}
}
