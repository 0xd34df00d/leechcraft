project (secman_securestorage)

LC_DEFINE_PLUGIN (
	SRCS
		securestorage.cpp
		cryptosystem.cpp
		ciphertextformat.cpp
		settingswidget.cpp
		newpassworddialog.cpp
	SETTINGS securestoragesettings.xml
	QT_COMPONENTS Widgets
	LINK_LIBRARIES crypto
	HAS_TESTS
	)

AddSecmanSecurestorageTest (cryptosystem SRCS tests/cryptosystemtest.cpp cryptosystem.cpp ciphertextformat.cpp LINK_LIBRARIES crypto)
