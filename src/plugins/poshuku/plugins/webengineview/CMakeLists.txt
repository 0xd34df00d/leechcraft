include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (WEBENGINEVIEW_SRCS
	cookiessyncer.cpp
	customwebview.cpp
	requestinterceptor.cpp
	webengineview.cpp
	)
set (WEBENGINEVIEW_FORMS
	)
set (WEBENGINEVIEW_RESOURCES)
#CreateTrs ("poshuku_webengineview" "en;ru_RU" WEBENGINEVIEW_COMPILED_TRANSLATIONS)
#CreateTrsUpTarget("poshuku_webengineview" "en;ru_RU" "${WEBENGINEVIEW_SRCS}" "${WEBENGINEVIEW_FORMS}" "poshukuwebengineviewsettings.xml")

QtWrapUi (WEBENGINEVIEW_UIS_H ${WEBENGINEVIEW_FORMS})
QtAddResources (WEBENGINEVIEW_RCCS ${WEBENGINEVIEW_RESOURCES})
add_library (leechcraft_poshuku_webengineview SHARED
	${WEBENGINEVIEW_COMPILED_TRANSLATIONS}
	${WEBENGINEVIEW_SRCS}
	${WEBENGINEVIEW_UIS_H}
	${WEBENGINEVIEW_RCCS}
	)
target_link_libraries (leechcraft_poshuku_webengineview
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_webengineview DESTINATION ${LC_PLUGINS_DEST})
#install (FILES poshukuwebengineviewsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_poshuku_webengineview PrintSupport WebEngineWidgets)
