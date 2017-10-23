cmake_minimum_required (VERSION 3.3)
project (leechcraft_otlozhu)
include (InitLCPlugin OPTIONAL)

option (ENABLE_OTLOZHU_SYNC "Enable synchronization support for Otlozhu" ON)

if (NOT ENABLE_OTLOZHU_SYNC)
	add_definitions (-DDISABLE_SYNC=1)
endif ()

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
	editcommentdialog.cpp
	editdatedialog.cpp
	xmlsettingsmanager.cpp
	)

set (LARETZ_LIB)
if (ENABLE_OTLOZHU_SYNC)
	set (SRCS
		${SRCS}
		syncproxy.cpp
		stager.cpp
		stagerhandler.cpp
		)
	set (LARETZ_LIB laretz_ops)
endif ()

set (FORMS
	todotab.ui
	addtododialog.ui
	itemsmergedialog.ui
	editcommentdialog.ui
	editdatedialog.ui
	)
set (RESOURCES otlozhuresources.qrc)

CreateTrs ("otlozhu" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("otlozhu" "en;ru_RU" "${SRCS}" "${FORMS}" "otlozhusettings.xml")

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})
add_definitions (-DBOOST_SPIRIT_USE_PHOENIX_V3)

add_library (leechcraft_otlozhu SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_otlozhu
	${LEECHCRAFT_LIBRARIES}
	${LARETZ_LIB}
	)
install (TARGETS leechcraft_otlozhu DESTINATION ${LC_PLUGINS_DEST})
install (FILES otlozhusettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_otlozhu Widgets Xml)
