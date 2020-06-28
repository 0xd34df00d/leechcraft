project (leechcraft_poshuku_qrd)

find_package (PkgConfig)
pkg_check_modules (QREncode REQUIRED IMPORTED_TARGET libqrencode)

LC_DEFINE_PLUGIN (
	SRCS qrd.cpp
	QT_COMPONENTS Widgets
	LINK_LIBRARIES PkgConfig::QREncode
	)
