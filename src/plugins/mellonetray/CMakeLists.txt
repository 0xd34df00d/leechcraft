CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_mellonetray)
INCLUDE (InitLCPlugin OPTIONAL)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	mellonetray.cpp
	)
#CreateTrs("mellonetray" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("mellonetray" "en;ru_RU" "${SRCS}" "${FORMS}" "")

ADD_LIBRARY (leechcraft_mellonetray SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_mellonetray
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_mellonetray DESTINATION ${LC_PLUGINS_DEST})
