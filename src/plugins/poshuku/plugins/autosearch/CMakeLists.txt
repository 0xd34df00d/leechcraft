include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (AUTOSEARCH_SRCS
	autosearch.cpp
	)
set (AUTOSEARCH_RESOURCES
	)
CreateTrs ("poshuku_autosearch" "en;ru_RU" AUTOSEARCH_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_autosearch" "en;ru_RU" "${AUTOSEARCH_SRCS}" "" "")

QT4_ADD_RESOURCES (AUTOSEARCH_RCCS ${AUTOSEARCH_RESOURCES})
add_library (leechcraft_poshuku_autosearch SHARED
	${AUTOSEARCH_COMPILED_TRANSLATIONS}
	${AUTOSEARCH_SRCS}
	${AUTOSEARCH_MOC_SRCS}
	${AUTOSEARCH_RCCS}
	)
target_link_libraries (leechcraft_poshuku_autosearch
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_autosearch DESTINATION ${LC_PLUGINS_DEST})
