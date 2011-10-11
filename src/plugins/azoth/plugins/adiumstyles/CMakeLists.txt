INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)
SET (ADIUMSTYLES_SRCS
	adiumstyles.cpp
	adiumstylesource.cpp
	packproxymodel.cpp
	)
SET (ADIUMSTYLES_HEADERS
	adiumstyles.h
	adiumstylesource.h
	packproxymodel.h
	)
QT4_WRAP_CPP (ADIUMSTYLES_MOC_SRCS ${ADIUMSTYLES_HEADERS})

ADD_LIBRARY (leechcraft_azoth_adiumstyles SHARED
	${ADIUMSTYLES_SRCS}
	${ADIUMSTYLES_MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_adiumstyles
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth_adiumstyles DESTINATION ${LC_PLUGINS_DEST})
INSTALL (DIRECTORY share/azoth DESTINATION share/leechcraft)