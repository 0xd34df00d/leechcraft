cmake_minimum_required (VERSION 2.8)
project (leechcraft_kinotify)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTWEBKIT TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	kinotify.cpp
	kinotifywidget.cpp
	notificationaction.cpp
	xmlsettingsmanager.cpp
	)

if (UNIX AND NOT APPLE)
	set (SRCS fswinwatcher_x11.cpp ${SRCS})
elseif (WIN32)
	set (SRCS fswinwatcher_win.cpp ${SRCS})
else ()
	set (SRCS fswinwatcher.cpp ${SRCS})
endif ()

set (RESOURCES kinotifyresources.qrc)
CreateTrs ("kinotify" "ar;de;en;es;fr;it;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("kinotify" "en;ru_RU" "${SRCS}" "${FORMS}" "kinotifysettings.xml")

QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_kinotify SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_kinotify
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_kinotify DESTINATION ${LC_PLUGINS_DEST})
install (FILES kinotifysettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY themes DESTINATION ${LC_SHARE_DEST}/kinotify)
