SET (QT_USE_QTSCRIPT TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	)

SET (SRCS
    xmlsettingsdialog.cpp
	rangewidget.cpp
	filepicker.cpp
	radiogroup.cpp
	scripter.cpp
	typeregister.cpp
	dir.cpp
	file.cpp
	bytearray.cpp
	settings.cpp
	basesettingsmanager.cpp
	)
SET (HEADERS
    xmlsettingsdialog.h
	rangewidget.h
	filepicker.h
	radiogroup.h
	scripter.h
	typeregister.h
	dir.h
	file.h
	bytearray.h
	settings.h
	basesettingsmanager.h
	)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ADD_LIBRARY (xmlsettingsdialog SHARED ${SRCS} ${MOC_SRCS})
TARGET_LINK_LIBRARIES (xmlsettingsdialog
	${QT_LIBRARIES}
	)
SET_PROPERTY (TARGET xmlsettingsdialog PROPERTY SOVERSION 0.3.0)
INSTALL (TARGETS xmlsettingsdialog DESTINATION lib)
