include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	)
set (SRCS
	simplestorage.cpp
	)
set (HEADERS
	simplestorage.h
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif ()

set (SIMPLESTORAGE_RESOURCES simplestorageresources.qrc)
QT4_ADD_RESOURCES (SIMPLESTORAGE_RCCS ${SIMPLESTORAGE_RESOURCES})

add_library (leechcraft_secman_simplestorage SHARED
	${SIMPLESTORAGE_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (leechcraft_secman_simplestorage
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_secman_simplestorage DESTINATION ${LC_PLUGINS_DEST})
