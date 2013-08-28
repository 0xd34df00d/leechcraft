include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (FILESCHEME_SRCS
	filescheme.cpp
	schemereply.cpp
	)
set (FILESCHEME_HEADERS
	filescheme.h
	schemereply.h
	)
set (FILESCHEME_RESOURCES
	poshukufileschemeresources.qrc
	)
CreateTrs ("poshuku_filescheme" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" FILESCHEME_COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (FILESCHEME_MOC_SRCS ${FILESCHEME_HEADERS})
endif ()
QT4_ADD_RESOURCES (FILESCHEME_RCCS ${FILESCHEME_RESOURCES})
add_library (leechcraft_poshuku_filescheme SHARED
	${FILESCHEME_COMPILED_TRANSLATIONS}
	${FILESCHEME_SRCS}
	${FILESCHEME_MOC_SRCS}
	${FILESCHEME_RCCS}
	)
target_link_libraries (leechcraft_poshuku_filescheme
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_filescheme RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${FILESCHEME_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
