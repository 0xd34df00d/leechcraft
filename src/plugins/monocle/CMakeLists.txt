CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_monocle)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	monocle.cpp
	documenttab.cpp
	core.cpp
	pagegraphicsitem.cpp
	filewatcher.cpp
	tocwidget.cpp
	presenterwidget.cpp
	pagesview.cpp
	xmlsettingsmanager.cpp
	pixmapcachemanager.cpp
	recentlyopenedmanager.cpp
	choosebackenddialog.cpp
	defaultbackendmanager.cpp
	docstatemanager.cpp
	docinfodialog.cpp
	bookmarkswidget.cpp
	bookmarksmanager.cpp
	bookmark.cpp
	thumbswidget.cpp
	pageslayoutmanager.cpp
	)
SET (FORMS
	documenttab.ui
	tocwidget.ui
	choosebackenddialog.ui
	docinfodialog.ui
	bookmarkswidget.ui
	thumbswidget.ui
	)
SET (RESOURCES monocleresources.qrc)
CreateTrs ("monocle" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("monocle" "en;ru_RU" "${SRCS}" "${FORMS}" "monoclesettings.xml")
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_monocle SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_monocle
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_monocle DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES monoclesettings.xml DESTINATION ${LC_SETTINGS_DEST})

OPTION (ENABLE_MONOCLE_FXB "Enable FictionBook backend for Monocle" ON)
OPTION (ENABLE_MONOCLE_MU "Enable PDF backend for Monocle using the mupdf library" OFF)
OPTION (ENABLE_MONOCLE_PDF "Enable PDF backend for Monocle using the Poppler library" ON)
OPTION (ENABLE_MONOCLE_POSTRUS "Enable PostScript backend for Monocle using the libspectre library" ON)
OPTION (ENABLE_MONOCLE_SEEN "Enable DjVu backend for Monocle using the DjVu library" ON)

IF (ENABLE_MONOCLE_FXB)
	ADD_SUBDIRECTORY (plugins/fxb)
ENDIF (ENABLE_MONOCLE_FXB)

IF (ENABLE_MONOCLE_MU)
	ADD_SUBDIRECTORY (plugins/mu)
ENDIF (ENABLE_MONOCLE_MU)

IF (ENABLE_MONOCLE_PDF)
	ADD_SUBDIRECTORY (plugins/pdf)
ENDIF (ENABLE_MONOCLE_PDF)

IF (ENABLE_MONOCLE_POSTRUS)
	ADD_SUBDIRECTORY (plugins/postrus)
ENDIF (ENABLE_MONOCLE_POSTRUS)

IF (ENABLE_MONOCLE_SEEN)
	ADD_SUBDIRECTORY (plugins/seen)
ENDIF (ENABLE_MONOCLE_SEEN)
