INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
SET (FUA_SRCS
	fua.cpp
	settings.cpp
	changer.cpp
	)
SET (FUA_HEADERS
	fua.h
	settings.h
	changer.h
	)
SET (FUA_FORMS
	settings.ui
	changer.ui
	)
QT4_WRAP_CPP (FUA_MOC_SRCS ${FUA_HEADERS})
QT4_WRAP_UI (FUA_UIS_H ${FUA_FORMS})
ADD_LIBRARY (leechcraft_poshuku_fua SHARED
	${FUA_SRCS}
	${FUA_MOC_SRCS}
	${FUA_UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_fua
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_poshuku_fua DESTINATION ${LC_PLUGINS_DEST})
