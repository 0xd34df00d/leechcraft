set (QML_SRCS
	colorthemeproxy.cpp
	settableiconprovider.cpp
	standardnamfactory.cpp
	themeimageprovider.cpp
	tooltipitem.cpp
	unhidelistmodel.cpp
	unhidelistviewbase.cpp
	util.cpp
	widthiconprovider.cpp
	)

if (WITH_QWT)
	find_package (PkgConfig)
	pkg_search_module (Qwt REQUIRED IMPORTED_TARGET Qt${LC_QT_VERSION}Qwt6 Qt${LC_QT_VERSION}Qwt)
	set (ADDITIONAL_LIBS PkgConfig::Qwt)
	add_definitions (-DWITH_QWT)

	set (QML_SRCS ${QML_SRCS} plotitem.cpp)
endif ()

add_util_library (util-qml
	SOURCES ${QML_SRCS}
	DEPENDS
		leechcraft-util-gui
		leechcraft-util-network
		leechcraft-util-sll
		leechcraft-util-sys
		${ADDITIONAL_LIBS}
	USES Network Quick QuickWidgets
	)
