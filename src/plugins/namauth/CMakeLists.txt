cmake_minimum_required (VERSION 3.3)
project (leechcraft_namauth)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	authenticationdialog.cpp
	namauth.cpp
	namhandler.cpp
	sqlstoragebackend.cpp
	)
set (FORMS
	authenticationdialog.ui
	)
CreateTrs("namauth" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("namauth" "en;ru_RU" "${SRCS}" "${FORMS}" "")

QtWrapUi (UIS_H ${FORMS})

add_library (leechcraft_namauth SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_namauth
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_namauth DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_namauth Network Sql Widgets)
