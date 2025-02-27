SECTION = "Wireless"

LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://COPYING;md5=5ebcb90236d1ad640558c3d3cd3035df \
                    file://README;beginline=1;endline=56;md5=e3d2f6c2948991e37c1ca4960de84747 \
                    file://wpa_supplicant/wpa_supplicant.c;beginline=1;endline=12;md5=76306a95306fee9a976b0ac1be70f705"

DEPENDS = "libnl"
RRECOMMENDS_${PN} = "${PN}-passphrase ${PN}-cli"

PACKAGECONFIG ??= "openssl"
PACKAGECONFIG[gnutls] = ",,gnutls libgcrypt"
PACKAGECONFIG[openssl] = ",,openssl"

inherit pkgconfig systemd

SYSTEMD_SERVICE_${PN} = "wpa_supplicant.service wpa_supplicant-nl80211@.service wpa_supplicant-wired@.service"
SYSTEMD_AUTO_ENABLE = "disable"

SUMMIT_URI ?= "${SUMMIT_URI_BASE}"
SUMMIT_URI_summit-internal = "${SUMMIT_URI_BASE}/summit_supplicant/laird/${PV}"

SRC_URI += "${SUMMIT_URI}/summit_supplicant-src-${PV}.tar.gz;name=summit-supplicant-src"

S = "${WORKDIR}/summit_supplicant-${PV}"

RPROVIDES_${PN} += "wpa-supplicant"
RREPLACES_${PN} += "wpa-supplicant"
RCONFLICTS_${PN} += "wpa-supplicant"

PROVIDES_${PN} += "wpa-supplicant-dbg"
RREPLACES_${PN} += "wpa-supplicant-dbg"
RCONFLICTS_${PN} += "wpa-supplicant-dbg"

RPROVIDES_${PN}-passphrase  += "wpa-supplicant-passphrase"
RREPLACES_${PN}-passphrase  += "wpa-supplicant-passphrase"
RCONFLICTS_${PN}-passphrase += "wpa-supplicant-passphrase"

RPROVIDES_${PN}-cli  += "wpa-supplicant-cli"
RREPLACES_${PN}-cli  += "wpa-supplicant-cli"
RCONFLICTS_${PN}-cli += "wpa-supplicant-cli"

PACKAGES_prepend = "${PN}-passphrase ${PN}-cli "
FILES_${PN}-passphrase = "${bindir}/wpa_passphrase"
FILES_${PN}-cli = "${sbindir}/wpa_cli"
FILES_${PN} += "${systemd_unitdir}/system/*"

SOLIBS = ".so*"
FILES_SOLIBSDEV = ""

do_configure () {
	cp -f wpa_supplicant/${DEFCONFIG} wpa_supplicant/.config

	if echo "${PACKAGECONFIG}" | grep -qw "openssl"; then
		ssl=openssl
	elif echo "${PACKAGECONFIG}" | grep -qw "gnutls"; then
		ssl=gnutls
	fi
	if [ -n "$ssl" ]; then
		sed -i "s/%ssl%/$ssl/" wpa_supplicant/.config
	fi

	# For rebuild
	rm -f wpa_supplicant/*.d wpa_supplicant/dbus/*.d
}

export EXTRA_CFLAGS = "${CFLAGS}"
export BINDIR = "${sbindir}"

EXTRA_OEMAKE = "CONFIG_SDC_RADIO_QCA45N=y CONFIG_DRIVER_NL80211=y"

do_compile () {
	unset CFLAGS CPPFLAGS CXXFLAGS
	sed -e "s:CFLAGS\ =.*:& \$(EXTRA_CFLAGS):g" -i ${S}/src/lib.rules
	oe_runmake -C wpa_supplicant
}

do_install () {
	install -D -m 755 wpa_supplicant/wpa_supplicant ${D}${sbindir}/sdcsupp
	install -D -t ${D}${sbindir} -m 755 wpa_supplicant/wpa_cli
	install -D -t ${D}${bindir} -m 755 wpa_supplicant/wpa_passphrase

	if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
		install -D -t ${D}${systemd_unitdir}/system -m 644 wpa_supplicant/systemd/*.service
	fi
}

pkg_postinst_${PN} () {
	# If we're offline, we don't need to do this.
	if [ "x$D" = "x" ]; then
		killall -q -HUP dbus-daemon || true
	fi
}
