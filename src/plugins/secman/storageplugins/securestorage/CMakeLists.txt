INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	)

SET (SRCS
	securestorage.cpp
	cryptosystem.cpp
	ciphertextformat.cpp
	)
SET (HEADERS
	securestorage.h
	cryptosystem.h
	ciphertextformat.h
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)

IF (TESTS_SECMAN)
	INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}/tests)
	QT4_WRAP_CPP (SECMAN_SECURESTORAGE_MOC "tests/cryptosystemtest.h")
	ADD_EXECUTABLE (lc_secman_securestorage_test WIN32
		tests/cryptosystemtest.cpp
		cryptosystem.cpp
		ciphertextformat.cpp
		${SECMAN_SECURESTORAGE_MOC}
	)
	TARGET_LINK_LIBRARIES (lc_secman_securestorage_test
		${QT_LIBRARIES}
		${LEECHCRAFT_LIBRARIES}
		crypto
	)
	ADD_TEST (CryptoSystemTest lc_secman_securestorage_test)
ENDIF (TESTS_SECMAN)

ADD_LIBRARY (leechcraft_secman_securestorage SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_secman_securestorage
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	crypto
	)
INSTALL (TARGETS leechcraft_secman_securestorage DESTINATION ${LC_PLUGINS_DEST})
