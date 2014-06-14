include_directories (${CMAKE_CURRENT_BINARY_DIR})
set (GUI_SRCS
	autoresizemixin.cpp
	clearlineeditaddon.cpp
	colorbutton.cpp
	findnotification.cpp
	findnotificationwk.cpp
	flowlayout.cpp
	lineeditbuttonmanager.cpp
	pagenotification.cpp
	selectablebrowser.cpp
	unhoverdeletemixin.cpp
	util.cpp
	)
set (GUI_FORMS
	findnotification.ui
	)

if (USE_QT5)
	qt5_wrap_ui (GUI_UIS_H ${GUI_FORMS})
else ()
	QT4_WRAP_UI (GUI_UIS_H ${GUI_FORMS})
endif ()

foreach (SRC ${GUI_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/gui/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

foreach (FORM ${GUI_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/gui/${FORM}")
endforeach ()
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_library (leechcraft-util-gui SHARED
	${GUI_SRCS}
	${GUI_UIS_H}
	)
target_link_libraries (leechcraft-util-gui
	${QT_LIBRARIES}
	leechcraft-util-network
	)
set_property (TARGET leechcraft-util-gui PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-gui DESTINATION ${LIBDIR})

if (USE_QT5)
	QT5_USE_MODULES (leechcraft-util-gui Network Widgets)
endif ()
