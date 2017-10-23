cmake_minimum_required (VERSION 3.3)
project (leechcraft_wkplugins)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (STATIC_SRCS
	staticplugin.cpp
	)
add_library (leechcraft_wkplugins_qtplugin STATIC
	${STATIC_SRCS}
	)
set_target_properties (leechcraft_wkplugins_qtplugin PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN")
FindQtLibs (leechcraft_wkplugins_qtplugin WebKitWidgets)

set (SRCS
	wkplugins.cpp
	notificationsext.cpp
	spellcheckext.cpp
	xmlsettingsmanager.cpp
	)
CreateTrs("wkplugins" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("wkplugins" "en;ru_RU" "${SRCS}" "${FORMS}" "wkpluginssettings.xml")

find_package (Qt5Core REQUIRED)
get_target_property  (QMAKE_EXECUTABLE Qt5::qmake LOCATION)

function(QUERY_QMAKE VAR RESULT)
	exec_program(${QMAKE_EXECUTABLE} ARGS "-query ${VAR}" RETURN_VALUE return_code OUTPUT_VARIABLE output )
	if (NOT return_code)
		file(TO_CMAKE_PATH "${output}" output)
		set(${RESULT} ${output} PARENT_SCOPE)
	endif ()
endfunction(QUERY_QMAKE)

query_qmake(QT_INSTALL_HEADERS HEADERS_PATH)

QT5_WRAP_CPP (WRAPPED_QHEADERS ${HEADERS_PATH}/QtWebKit/qwebkitplatformplugin.h)

add_library (leechcraft_wkplugins SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${WRAPPED_QHEADERS}
	)
target_link_libraries (leechcraft_wkplugins
	${LEECHCRAFT_LIBRARIES}
	leechcraft_wkplugins_qtplugin
	)
install (TARGETS leechcraft_wkplugins DESTINATION ${LC_PLUGINS_DEST})
install (FILES wkpluginssettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_wkplugins WebKitWidgets)
