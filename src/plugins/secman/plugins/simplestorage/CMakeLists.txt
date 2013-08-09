INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	)
SET (SRCS
	simplestorage.cpp
	)
SET (HEADERS
	simplestorage.h
	)

SET (SIMPLESTORAGE_RESOURCES simplestorageresources.qrc)
QtAddResources (SIMPLESTORAGE_RCCS ${SIMPLESTORAGE_RESOURCES})

ADD_LIBRARY (leechcraft_secman_simplestorage SHARED
	${SIMPLESTORAGE_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)

FindQtLibs (leechcraft_secman_simplestorage Core)

TARGET_LINK_LIBRARIES (leechcraft_secman_simplestorage
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_secman_simplestorage DESTINATION ${LC_PLUGINS_DEST})
