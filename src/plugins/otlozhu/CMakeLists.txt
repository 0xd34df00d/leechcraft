cmake_minimum_required (VERSION 2.8)
project (leechcraft_otlozhu)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	otlozhu.cpp
	todotab.cpp
	core.cpp
	todomanager.cpp
	todoitem.cpp
	todostorage.cpp
	storagemodel.cpp
	addtododialog.cpp
	todolistdelegate.cpp
	todosfproxymodel.cpp
	notificationsmanager.cpp
	icalgenerator.cpp
	icalparser.cpp
	itemsmergedialog.cpp
	syncproxy.cpp
	stager.cpp
	stagerhandler.cpp
	)
set (FORMS
	todotab.ui
	addtododialog.ui
	itemsmergedialog.ui
	)
set (RESOURCES otlozhuresources.qrc)
CreateTrs ("otlozhu" "en;ru_RU" COMPILED_TRANSLATIONS)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
add_definitions (-DBOOST_SPIRIT_USE_PHOENIX_V3)

add_library (leechcraft_otlozhu SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_otlozhu
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	laretz_ops
	)
install (TARGETS leechcraft_otlozhu DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
