cmake_minimum_required (VERSION 3.3)
project (leechcraft_historyholder)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	historyholder.cpp
	findproxy.cpp
	historydb.cpp
	historyentry.cpp
	)
set (RESOURCES historyholderresources.qrc)
CreateTrs ("historyholder" "ar;de;en;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("historyholder" "en;ru_RU" "${SRCS}" "" "")

QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_historyholder SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_historyholder
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_historyholder DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_historyholder Concurrent Gui Sql)
