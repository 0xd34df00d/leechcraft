cmake_minimum_required (VERSION 3.3)
project (leechcraft_lmp_mtpsync)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (MTP REQUIRED IMPORTED_TARGET libmtp)

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (MTPSYNC_SRCS
	mtpsync.cpp
	)

add_library (leechcraft_lmp_mtpsync SHARED
	${MTPSYNC_COMPILED_TRANSLATIONS}
	${MTPSYNC_SRCS}
	)
target_link_libraries (leechcraft_lmp_mtpsync
	${LEECHCRAFT_LIBRARIES}
	PkgConfig::MTP
	)

install (TARGETS leechcraft_lmp_mtpsync DESTINATION ${LC_PLUGINS_DEST})
#install (FILES lmpmtpsyncsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_lmp_mtpsync Concurrent Gui)
