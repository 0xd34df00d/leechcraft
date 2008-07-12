INCLUDE_DIRECTORIES(
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_BINARY_DIR}
   .
   src/
)
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
