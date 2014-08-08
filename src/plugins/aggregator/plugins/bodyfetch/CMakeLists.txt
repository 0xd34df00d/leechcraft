include_directories (${AGGREGATOR_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)
set (BODYFETCH_SRCS
	bodyfetch.cpp
	workerobject.cpp
	)

set (BODYFETCH_RESOURCES bodyfetchresources.qrc)
QtAddResources (BODYFETCH_RCCS ${BODYFETCH_RESOURCES})

add_library (leechcraft_aggregator_bodyfetch SHARED
	${BODYFETCH_RCCS}
	${BODYFETCH_COMPILED_TRANSLATIONS}
	${BODYFETCH_SRCS}
	)
target_link_libraries (leechcraft_aggregator_bodyfetch
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_aggregator_bodyfetch DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/scripts DESTINATION ${LC_SHARE_DEST})

FindQtLibs (leechcraft_aggregator_bodyfetch WebKitWidgets)
