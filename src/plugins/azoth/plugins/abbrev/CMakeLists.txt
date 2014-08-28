cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_abbrev)
include (InitLCPlugin OPTIONAL)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (ABBREV_SRCS
	abbrev.cpp
	)

add_library (leechcraft_azoth_abbrev SHARED
	${ABBREV_SRCS}
	)
target_link_libraries (leechcraft_azoth_abbrev
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_abbrev DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_azoth_abbrev Core)
