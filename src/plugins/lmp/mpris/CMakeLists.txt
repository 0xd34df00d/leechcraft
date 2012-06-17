SET (QT_USE_QTDBUS TRUE)
INCLUDE (${QT_USE_FILE})

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

SET (SRCS
	instance.cpp
	mediaplayer2adaptor.cpp
	playeradaptor.cpp
	fdopropsadaptor.cpp
	)
SET (HEADERS
	instance.h
	mediaplayer2adaptor.h
	playeradaptor.h
	fdopropsadaptor.h
	)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)

SET (MPRIS_SUBLIB leechcraft_lmp_mpris PARENT_SCOPE)
SET (MPRIS_SUBLIB leechcraft_lmp_mpris)
ADD_LIBRARY (${MPRIS_SUBLIB} STATIC
	${SRCS}
	${MOC_SRCS}
	)