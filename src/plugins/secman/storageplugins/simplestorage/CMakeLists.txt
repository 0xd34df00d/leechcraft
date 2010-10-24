INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	)
SET (SRCS
	simplestorage.cpp
	)
SET (HEADERS
	simplestorage.h
	)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})

ADD_LIBRARY (leechcraft_secman_simplestorage SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_secman_simplestorage
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_secman_simplestorage DESTINATION ${LC_PLUGINS_DEST})
