if (WITH_QWT)
	find_package (Qwt REQUIRED)
	set (ADDITIONAL_LIBS ${QWT_LIBRARIES})
	add_definitions (-DWITH_QWT)

	include_directories (${QWT_INCLUDE_DIRS})
endif ()

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
	qmlerrorwatcher.cpp
	)

if (WITH_QWT)
	set (QML_SRCS ${QML_SRCS} plotitem.cpp)
endif ()

foreach (SRC ${QML_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/qml/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-qml${LC_LIBSUFFIX} SHARED
	${QML_SRCS}
	)
target_link_libraries (leechcraft-util-qml${LC_LIBSUFFIX}
	${ADDITIONAL_LIBS}
	leechcraft-util-gui${LC_LIBSUFFIX}
	leechcraft-util-network${LC_LIBSUFFIX}
	leechcraft-util-sll${LC_LIBSUFFIX}
	leechcraft-util-sys${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-qml${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION}.2)
install (TARGETS leechcraft-util-qml${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-qml${LC_LIBSUFFIX} Network Quick QuickWidgets)
