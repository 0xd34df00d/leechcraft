cmake_minimum_required (VERSION 3.3)
project (leechcraft_zalil)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	bitcheesepending.cpp
	bitcheeseservice.cpp
	pendinguploadbase.cpp
	servicebase.cpp
	servicesmanager.cpp
	zalil.cpp
	)
CreateTrs("zalil" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("zalil" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_zalil SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_zalil
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_zalil DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_zalil Network)
