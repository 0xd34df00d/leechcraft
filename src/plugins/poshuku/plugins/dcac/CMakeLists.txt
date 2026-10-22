project (poshuku_dcac)

LC_DEFINE_PLUGIN (
	SRCS
		dcac.cpp
		effectprocessor.cpp
		effects.cpp
		viewsmanager.cpp
		invertcolors.cpp
		reducelightness.cpp
		colortemp.cpp
		scriptobject.cpp
		scripthandler.cpp
	SETTINGS poshukudcacsettings.xml
	QT_COMPONENTS Widgets WebKitWidgets
	HAS_TESTS
	)

foreach (_test getgray reducelightness invertrgb temp2rgb colortemp)
	AddPoshukuDcacTest (${_test} SRCS tests/${_test}test.cpp tests/testbase.cpp QT_COMPONENTS Concurrent)
endforeach ()
