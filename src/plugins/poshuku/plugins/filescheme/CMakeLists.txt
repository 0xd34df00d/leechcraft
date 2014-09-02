include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (FILESCHEME_SRCS
	filescheme.cpp
	schemereply.cpp
	)
set (FILESCHEME_RESOURCES
	poshukufileschemeresources.qrc
	)
CreateTrs ("poshuku_filescheme" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" FILESCHEME_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_filescheme" "en;ru_RU" "${FILESCHEME_SRCS}" "${FILESCHEME_FORMS}" "")

QtAddResources (FILESCHEME_RCCS ${FILESCHEME_RESOURCES})
add_library (leechcraft_poshuku_filescheme SHARED
	${FILESCHEME_COMPILED_TRANSLATIONS}
	${FILESCHEME_SRCS}
	${FILESCHEME_RCCS}
	)
target_link_libraries (leechcraft_poshuku_filescheme
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_filescheme DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_poshuku_filescheme Network)
