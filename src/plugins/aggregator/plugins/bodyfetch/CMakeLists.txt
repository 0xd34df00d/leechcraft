include_directories (${AGGREGATOR_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)
set (BODYFETCH_SRCS
	bodyfetch.cpp
	workerobject.cpp
	)
set (BODYFETCH_HEADERS
	bodyfetch.h
	workerobject.h
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (BODYFETCH_MOC_SRCS ${BODYFETCH_HEADERS})
endif ()

set (BODYFETCH_RESOURCES bodyfetchresources.qrc)
QT4_ADD_RESOURCES (BODYFETCH_RCCS ${BODYFETCH_RESOURCES})

add_library (leechcraft_aggregator_bodyfetch SHARED
	${BODYFETCH_RCCS}
	${BODYFETCH_COMPILED_TRANSLATIONS}
	${BODYFETCH_SRCS}
	${BODYFETCH_MOC_SRCS}
	)
target_link_libraries (leechcraft_aggregator_bodyfetch
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_aggregator_bodyfetch RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/scripts DESTINATION ${LC_SHARE_DEST})