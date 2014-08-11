set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set (SRCS
	instance.cpp
	mediaplayer2adaptor.cpp
	playeradaptor.cpp
	fdopropsadaptor.cpp
	)

set (MPRIS_SUBLIB leechcraft_lmp_mpris PARENT_SCOPE)
set (MPRIS_SUBLIB leechcraft_lmp_mpris)
add_library (${MPRIS_SUBLIB} STATIC
	${SRCS}
	)
target_link_libraries (${MPRIS_SUBLIB}
	${QT_LIBRARIES}
	)

FindQtLibs (leechcraft_lmp_mpris DBus)
