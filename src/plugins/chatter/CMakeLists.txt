INCLUDE_DIRECTORIES(
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_BINARY_DIR}
   ../..
)

SET(fsirc3_SRCS
fsirc.cpp
chatter.cpp
irc.cpp
fsettings.cpp
)

SET(fsirc3_MOC_HDRS
config.h
fsirc.h
irc.h
chatter.h
fsettings.h
)

SET(fsirc3_UIS
    fsmain.ui
)

QT4_WRAP_UI(fsirc3_UIS_H ${fsirc3_UIS})

QT4_WRAP_CPP(fsirc3_MOC_SRCS ${fsirc3_MOC_HDRS})
ADD_LIBRARY(leechcraft_chatter SHARED ${fsirc3_SRCS} ${fsirc3_MOC_SRCS} ${fsirc3_UIS_H})
TARGET_LINK_LIBRARIES(leechcraft_chatter
   ${QT_LIBRARIES}
   plugininterface
)
INSTALL(TARGETS leechcraft_chatter DESTINATION share/leechcraft/plugins)
