cmake_minimum_required (VERSION 3.3)
project (leechcraft_advancednotifications_dolle)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (DOLLE_SRCS
	dolle.cpp
	notificationhandler.cpp
	dockutil.mm
	lcbadgeview.mm
	)
#CreateTrs ("advancednotifications_dolle" "en;ru_RU" DOLLE_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("advancednotifications_dolle" "en;ru_RU" "${DOLLE_SRCS}" "${DOLLE_FORMS}" "")

add_library (leechcraft_advancednotifications_dolle SHARED
	${DOLLE_COMPILED_TRANSLATIONS}
	${DOLLE_SRCS}
	)
target_link_libraries (leechcraft_advancednotifications_dolle
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	"-framework AppKit"
	)

install (TARGETS leechcraft_advancednotifications_dolle DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_advancednotifications_dolle Gui)
