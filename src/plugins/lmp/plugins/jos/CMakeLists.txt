cmake_minimum_required (VERSION 3.3)
project (leechcraft_lmp_jos)
include (InitLCPlugin)

pkg_check_modules (LIBGPOD libgpod-1.0)
pkg_check_modules (LIBIMOBILEDEVICE libimobiledevice-1.0)

message (STATUS "Found libimobiledevice cflags: ${LIBIMOBILEDEVICE_CFLAGS}")
message (STATUS "Found libimobiledevice libs at ${LIBIMOBILEDEVICE_LIBRARIES}")
message (STATUS "Found gpod cflags: ${LIBGPOD_CFLAGS}")
message (STATUS "Found gpod libs at ${LIBGPOD_LIBRARIES}")

add_definitions (${LIBIMOBILEDEVICE_CFLAGS})
add_definitions (${LIBGPOD_CFLAGS})

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (JOS_SRCS
	jos.cpp
	mobileraii.cpp
	devmanager.cpp
	connection.cpp
	uploadmanager.cpp
	afcfile.cpp
	gpoddb.cpp
	)

#CreateTrs ("lmp_jos" "en;ru_RU" JOS_COMPILED_TRANSLATIONS)
#CreateTrsUpTarget("lmp_jos" "en;ru_RU" "${JOS_SRCS}" "${JOS_FORMS}" "lmpjossettings.xml")

add_library (leechcraft_lmp_jos SHARED
	${JOS_COMPILED_TRANSLATIONS}
	${JOS_SRCS}
	)
target_link_libraries (leechcraft_lmp_jos
	${LEECHCRAFT_LIBRARIES}
	${LIBIMOBILEDEVICE_LIBRARIES}
	${LIBGPOD_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_jos DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_lmp_jos Concurrent Gui)
