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
	pkg_search_module (Qwt REQUIRED IMPORTED_TARGET Qt5Qwt Qt5Qwt6)
	set (ADDITIONAL_LIBS PkgConfig::Qwt)
	add_definitions (-DWITH_QWT)

	set (QML_SRCS ${QML_SRCS} plotitem.cpp)
endif ()

foreach (SRC ${QML_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/qml/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

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
