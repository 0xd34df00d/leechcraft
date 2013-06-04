set (QT_USE_QTDBUS TRUE)
include (${QT_USE_FILE})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set (SRCS
	instance.cpp
	mediaplayer2adaptor.cpp
	playeradaptor.cpp
	fdopropsadaptor.cpp
	)
set (HEADERS
	instance.h
	mediaplayer2adaptor.h
	playeradaptor.h
	fdopropsadaptor.h
	)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif (NOT LC_NO_MOC)

set (MPRIS_SUBLIB leechcraft_lmp_mpris PARENT_SCOPE)
set (MPRIS_SUBLIB leechcraft_lmp_mpris)
add_library (${MPRIS_SUBLIB} STATIC
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (${MPRIS_SUBLIB}
	${QT_LIBRARIES}
	)