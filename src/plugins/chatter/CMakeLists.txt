INCLUDE_DIRECTORIES(
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_BINARY_DIR}
   .
   src
)

SET(FSIRC_VERSION "3.1")
SET(FSIRC_VERSION_REPLY "tr(\"VERSION Leechcraft Chatter v%1, using Qt v %2\").arg(FS_VERSION,QT_VERSION_STR)")
# Dunno where.
SET(FSIRC_DEF_URI "irc://irc.freenode.net/#qt-ru")
SET(FSIRC_NICK "lcraft%1")
SET(FSIRC_IDENT "lcraft")
SET(FSIRC_REALNAME "tr(\"Leechcraft user\")")
SET(FSIRC_COM_CHAR "/")
SET(FSIRC_QUIT_MSG "tr(\"STOP DRINKING COGNAC ON MORNINGS!\")")


CONFIGURE_FILE("src/config.cmake" "config.h")

# ADD_DEFINITIONS("-DFSIRC_NO_TRAY_ICON")
# Hooray for icons!
INCLUDE(src/CMakeLists.txt)

SET(chatter_SRCS chatter.cpp)
SET(chatter_MOC_HDRS chatter.h)

QT4_WRAP_CPP(chatter_MOC_SRCS ${chatter_MOC_HDRS})

ADD_LIBRARY(leechcraft_chatter SHARED ${fsirc_SRCS} ${fsirc_MOC_SRCS} ${chatter_SRCS} ${chatter_MOC_SRCS} ${fsirc_UIS_H} ${fsirc_RES_H})
TARGET_LINK_LIBRARIES(leechcraft_chatter
   ${QT_LIBRARIES}
   plugininterface
)
INSTALL(TARGETS leechcraft_chatter DESTINATION ${LC_PLUGINS_DEST})
