project (leechcraft_secman_securestorage)

LC_DEFINE_PLUGIN (
	SRCS
		securestorage.cpp
		cryptosystem.cpp
		ciphertextformat.cpp
		settingswidget.cpp
		xmlsettingsmanager.cpp
		newpassworddialog.cpp
	SETTINGS securestoragesettings.xml
	QT_COMPONENTS Widgets
	LINK_LIBRARIES crypto
	)

if (TESTS_SECMAN)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests)
	add_executable (lc_secman_securestorage_test WIN32
		tests/cryptosystemtest.cpp
		cryptosystem.cpp
		ciphertextformat.cpp
		)
	target_link_libraries (lc_secman_securestorage_test
		${LEECHCRAFT_LIBRARIES}
		crypto
		)
	add_test (CryptoSystemTest lc_secman_securestorage_test)

	FindQtLibs (lc_secman_securestorage_test Test)
endif ()
