INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR} ../..)

SET (SRCS
	 generic.cpp
	 invalidparameter.cpp
	 logic.cpp
	 notimplemented.cpp
	 io.cpp
	 situative.cpp
	 outofbounds.cpp
	 )

ADD_LIBRARY (exceptions SHARED ${SRCS})
INSTALL (TARGETS exceptions DESTINATION lib)
