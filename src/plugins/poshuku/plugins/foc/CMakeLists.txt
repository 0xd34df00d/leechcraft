include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (FOC_SRCS
	foc.cpp
	xmlsettingsmanager.cpp
	)
set (FOC_RESOURCES
	)
#CreateTrs ("poshuku_foc" "en;ru_RU" FOC_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_foc" "en;ru_RU" "${FOC_SRCS}" "" "")

QtAddResources (FOC_RCCS ${FOC_RESOURCES})
add_library (leechcraft_poshuku_foc SHARED
	${FOC_COMPILED_TRANSLATIONS}
	${FOC_SRCS}
	${FOC_RCCS}
	)
target_link_libraries (leechcraft_poshuku_foc
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_foc DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_poshuku_foc Widgets WebKitWidgets)
