include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	)

set (SRCS
	securestorage.cpp
	cryptosystem.cpp
	ciphertextformat.cpp
	settingswidget.cpp
	xmlsettingsmanager.cpp
	newpassworddialog.cpp
	)
set (FORMS
	settingswidget.ui
	newpassworddialog.ui
	)
QT4_WRAP_UI (UIS_H ${FORMS})

if (TESTS_SECMAN)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests)
	QT4_WRAP_CPP (SECMAN_SECURESTORAGE_MOC "tests/cryptosystemtest.h")
	add_executable (lc_secman_securestorage_test WIN32
		tests/cryptosystemtest.cpp
		cryptosystem.cpp
		ciphertextformat.cpp
		${SECMAN_SECURESTORAGE_MOC}
	)
	target_link_libraries (lc_secman_securestorage_test
		${QT_LIBRARIES}
		${LEECHCRAFT_LIBRARIES}
		crypto
	)
	add_test (CryptoSystemTest lc_secman_securestorage_test)
endif ()

add_library (leechcraft_secman_securestorage SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_secman_securestorage
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	crypto
	)
install (TARGETS leechcraft_secman_securestorage DESTINATION ${LC_PLUGINS_DEST})
install (FILES securestoragesettings.xml DESTINATION ${LC_SETTINGS_DEST})
