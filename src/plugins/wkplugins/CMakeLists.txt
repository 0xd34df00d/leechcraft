cmake_minimum_required (VERSION 2.8)
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
install (TARGETS leechcraft_wkplugins_qtplugin DESTINATION ${LC_PLUGINS_DEST}/lib)
FindQtLibs (leechcraft_wkplugins_qtplugin WebKitWidgets)

set (SRCS
	wkplugins.cpp
	)
#CreateTrs("wkplugins" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("wkplugins" "en;ru_RU" "${SRCS}" "${FORMS}" "")

if (USE_QT5)
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
else ()
	find_package (Qt4 REQUIRED)
	set (HEADERS_PATH ${QT_INCLUDE_DIR})
endif ()

if (USE_QT5)
	QT5_WRAP_CPP (WRAPPED_QHEADERS ${HEADERS_PATH}/QtWebKit/qwebkitplatformplugin.h)
else ()
	QT4_WRAP_CPP (WRAPPED_QHEADERS ${HEADERS_PATH}/QtWebKit/qwebkitplatformplugin.h)
endif ()

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

FindQtLibs (leechcraft_wkplugins WebKitWidgets)
