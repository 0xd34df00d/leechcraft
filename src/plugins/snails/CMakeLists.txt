if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif ()

	project (leechcraft_snails)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif ()

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif ()

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (MAGIC)
if (MAGIC_FOUND)
	add_definitions (-DHAVE_MAGIC)
endif ()

set (QT_USE_QTWEBKIT TRUE)
set (QT_USE_QTSQL TRUE)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${MAGIC_INCLUDE_DIRS}
	)
set (SRCS
	snails.cpp
	core.cpp
	mailtab.cpp
	xmlsettingsmanager.cpp
	accountslistwidget.cpp
	account.cpp
	accountconfigdialog.cpp
	message.cpp
	accountthread.cpp
	accountthreadworker.cpp
	progresslistener.cpp
	storage.cpp
	progressmanager.cpp
	mailtreedelegate.cpp
	composemessagetab.cpp
	accountfoldermanager.cpp
	attdescr.cpp
	vmimeconversions.cpp
	outputiodevadapter.cpp
	mailmodelmanager.cpp
	common.cpp
	)
set (FORMS
	mailtab.ui
	accountslistwidget.ui
	accountconfigdialog.ui
	composemessagetab.ui
	)
set (RESOURCES snailsresources.qrc)
CreateTrs ("snails" "en;ru_RU" COMPILED_TRANSLATIONS)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_snails SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_snails
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${MAGIC_LIBRARIES}
	vmime
	)
install (TARGETS leechcraft_snails DESTINATION ${LC_PLUGINS_DEST})
install (FILES snailssettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/snails DESTINATION ${LC_SHARE_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
