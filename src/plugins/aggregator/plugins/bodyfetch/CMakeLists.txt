INCLUDE_DIRECTORIES (${AGGREGATOR_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)
SET (BODYFETCH_SRCS
	bodyfetch.cpp
	workerobject.cpp
	)
SET (BODYFETCH_HEADERS
	bodyfetch.h
	workerobject.h
	)

SET (BODYFETCH_RESOURCES bodyfetchresources.qrc)
QtAddResources (BODYFETCH_RCCS ${BODYFETCH_RESOURCES})

ADD_LIBRARY (leechcraft_aggregator_bodyfetch SHARED
	${BODYFETCH_RCCS}
	${BODYFETCH_COMPILED_TRANSLATIONS}
	${BODYFETCH_SRCS}
	)

FindQtLibs (leechcraft_aggregator_bodyfetch WebKitWidgets)

TARGET_LINK_LIBRARIES (leechcraft_aggregator_bodyfetch
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_aggregator_bodyfetch DESTINATION ${LC_PLUGINS_DEST})
INSTALL (DIRECTORY share/scripts DESTINATION ${LC_SHARE_DEST})
