SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

SET (SRCS
	SettingsNames.cpp
	)
SET (HEADERS
	PortNumbers.h
	SettingsNames.h
	SqlTransactionAutoAborter.h
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)

ADD_LIBRARY (Common STATIC
	${SRCS}
	${MOC_SRCS}
	)