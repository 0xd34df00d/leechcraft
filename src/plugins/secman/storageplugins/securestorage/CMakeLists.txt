INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	)
SET (SRCS
	securestorage.cpp
	cryptosystem.cpp
	)
SET (HEADERS
	securestorage.h
	cryptosystem.h
	)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})

ADD_LIBRARY (leechcraft_secman_securestorage SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_secman_securestorage
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_secman_securestorage DESTINATION ${LC_PLUGINS_DEST})
