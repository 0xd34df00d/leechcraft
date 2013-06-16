cmake_minimum_required (VERSION 2.8)
project (leechcraft_anhero_crashprocess)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	)
set (SRCS
	main.cpp
	gdblauncher.cpp
	)

add_executable (lc_anhero_crashprocess WIN32
	${SRCS}
	)
target_link_libraries (lc_anhero_crashprocess
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	)
install (TARGETS lc_anhero_crashprocess DESTINATION ${LC_BINDIR})
