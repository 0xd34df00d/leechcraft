cmake_minimum_required (VERSION 2.8)
project (leechcraft_historyholder)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	historyholder.cpp
	findproxy.cpp
	core.cpp
	)
set (HEADERS
	historyholder.h
	findproxy.h
	core.h
	)
set (RESOURCES historyholderresources.qrc)
CreateTrs ("historyholder" "ar;de;en;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)

QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_historyholder SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)

FindQtLibs (leechcraft_historyholder Core)

target_link_libraries (leechcraft_historyholder
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_historyholder DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
