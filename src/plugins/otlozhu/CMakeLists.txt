CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_otlozhu)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	otlozhu.cpp
	todotab.cpp
	core.cpp
	todomanager.cpp
	todoitem.cpp
	todostorage.cpp
	storagemodel.cpp
	addtododialog.cpp
	)
SET (HEADERS
	otlozhu.h
	todotab.h
	core.h
	todomanager.h
	todoitem.h
	todostorage.h
	storagemodel.h
	addtododialog.h
	)
SET (FORMS
	todotab.ui
	addtododialog.ui
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})

ADD_LIBRARY (leechcraft_otlozhu SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_otlozhu
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_otlozhu DESTINATION ${LC_PLUGINS_DEST})
