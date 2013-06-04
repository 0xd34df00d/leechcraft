if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif (COMMAND cmake_policy)

	project (leechcraft_shellopen)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif (NOT CMAKE_MODULE_PATH)

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif (NOT QT_USE_FILE)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	shellopen.cpp
	)
set (HEADERS
	shellopen.h
	)
set (TRANSLATIONS
	leechcraft_shellopen_uk_UA.ts
	leechcraft_shellopen_ru_RU.ts
	leechcraft_shellopen_en.ts
	)
set (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_shellopen_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_shellopen_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_shellopen_en.qm
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif (NOT LC_NO_MOC)

add_custom_command (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
add_library (leechcraft_shellopen SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (leechcraft_shellopen
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_shellopen DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
