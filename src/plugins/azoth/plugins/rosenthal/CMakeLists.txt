INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)
SET (ROSENTHAL_SRCS
	rosenthal.cpp
	highlighter.cpp
	)
SET (ROSENTHAL_HEADERS
	rosenthal.h
	highlighter.h
	)
QT4_WRAP_CPP (ROSENTHAL_MOC_SRCS ${ROSENTHAL_HEADERS})

FILE (GLOB R_HUNSPELL_SRCS "hunspell/*.hxx" "hunspell/*.cxx")

ADD_LIBRARY (leechcraft_azoth_rosenthal SHARED
    ${ROSENTHAL_SRCS}
    ${ROSENTHAL_MOC_SRCS}
	${R_HUNSPELL_SRCS}
    )
TARGET_LINK_LIBRARIES (leechcraft_azoth_rosenthal
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth_rosenthal DESTINATION ${LC_PLUGINS_DEST})
