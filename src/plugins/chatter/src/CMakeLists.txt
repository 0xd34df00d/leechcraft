INCLUDE_DIRECTORIES(
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_BINARY_DIR}
   .
   src
)

CONFIGURE_FILE(src/config.cmake src/config.h)

SET(fsirc_SRCS
src/fsirc.cpp
src/main.cpp
src/irc.cpp
src/fsettings.cpp
src/fscmdedit.cpp
src/fstrayicon.cpp
)

SET(fsirc_MOC_HDRS
src/fsirc.h
src/irc.h
src/fsettings.h
src/fscmdedit.h
src/fstrayicon.h
)

SET(fsirc_UIS
    src/fsmain.ui
)

SET(fsirc_RES
	src/fsirc.qrc
)

QT4_WRAP_UI(fsirc_UIS_H ${fsirc_UIS})
QT4_ADD_RESOURCES(fsirc_RES_H ${fsirc_RES})
QT4_WRAP_CPP(fsirc_MOC_SRCS ${fsirc_MOC_HDRS})
