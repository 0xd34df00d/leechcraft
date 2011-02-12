INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)
SET (CHATHISTORY_SRCS
	chathistory.cpp
	core.cpp
	storage.cpp
	chathistorywidget.cpp
	storagethread.cpp
	)
SET (CHATHISTORY_HEADERS
	chathistory.h
	core.h
	storage.h
	chathistorywidget.h
	storagethread.h
	)
SET (CHATHISTORY_FORMS
	chathistorywidget.ui
	)
QT4_WRAP_CPP (CHATHISTORY_MOC_SRCS ${CHATHISTORY_HEADERS})
QT4_WRAP_UI (CHATHISTORY_UIS_H ${CHATHISTORY_FORMS})

ADD_LIBRARY (leechcraft_azoth_chathistory SHARED
	${CHATHISTORY_SRCS}
	${CHATHISTORY_MOC_SRCS}
	${CHATHISTORY_UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_chathistory
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth_chathistory DESTINATION ${LC_PLUGINS_DEST})
