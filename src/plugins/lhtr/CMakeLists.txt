cmake_minimum_required (VERSION 3.10)
project (lhtr)
include (InitLCPlugin NO_POLICY_SCOPE)

option (WITH_LHTR_HTML "Build extended support for (invalid) HTML" ON)

if (WITH_LHTR_HTML)
	find_package (PkgConfig)
	pkg_check_modules (Tidy REQUIRED IMPORTED_TARGET tidy)
	set (TIDY_LIB PkgConfig::Tidy)
	add_definitions (-DWITH_HTMLTIDY)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		lhtr.cpp
		richeditorwidget.cpp
		hyperlinkdialog.cpp
		imagedialog.cpp
		finddialog.cpp
		inserttabledialog.cpp
		htmlhighlighter.cpp
		imagecollectiondialog.cpp
		imageinfosmodel.cpp
	SETTINGS lhtrsettings.xml
	QT_COMPONENTS Network WebEngineWidgets
	LINK_LIBRARIES ${TIDY_LIB}
	INSTALL_SHARE
	)
