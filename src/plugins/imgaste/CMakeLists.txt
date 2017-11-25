cmake_minimum_required (VERSION 3.3)
project (leechcraft_imgaste)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	imgaste.cpp
	hostingservice.cpp
	poster.cpp
	requestbuilder.cpp
	)
CreateTrs("imgaste" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("imgaste" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_imgaste SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_imgaste
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_imgaste DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_imgaste Network Widgets)
