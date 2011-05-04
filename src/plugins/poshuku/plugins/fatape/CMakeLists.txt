INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
SET (FATAPE_SRCS
	fatape.cpp
    userscript.cpp
	)
SET (FATAPE_HEADERS
	fatape.h
    userscript.h
	)

QT4_WRAP_CPP (FATAPE_MOC_SRCS ${FATAPE_HEADERS})
ADD_LIBRARY (leechcraft_poshuku_fatape SHARED
	${FATAPE_SRCS}
	${FATAPE_MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_fatape
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_poshuku_fatape DESTINATION ${LC_PLUGINS_DEST})
