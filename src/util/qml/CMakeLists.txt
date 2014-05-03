set (QML_SRCS
	colorthemeproxy.cpp
	settableiconprovider.cpp
	standardnamfactory.cpp
	themeimageprovider.cpp
	tooltipitem.cpp
	unhidelistmodel.cpp
	unhidelistviewbase.cpp
	widthiconprovider.cpp
	)

if (WITH_QWT)
	set (QML_SRCS ${QML_SRCS} plotitem.cpp)
endif ()

if (WITH_QWT)
	find_package (Qwt REQUIRED)
	set (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${QWT_LIBRARIES})
	add_definitions (-DWITH_QWT)
endif ()

foreach (SRC ${QML_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/qml/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-qml SHARED
	${QML_SRCS}
	)
target_link_libraries (leechcraft-util-qml
	${QT_LIBRARIES}
	${ADDITIONAL_LIBS}
	)
set_property (TARGET leechcraft-util-qml PROPERTY SOVERSION 0.6.65)
install (TARGETS leechcraft-util-qml DESTINATION ${LIBDIR})

if (USE_QT5)
	QT5_USE_MODULES (leechcraft-util-qml Declarative Network)
endif ()

