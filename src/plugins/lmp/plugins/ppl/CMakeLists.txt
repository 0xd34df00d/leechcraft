cmake_minimum_required (VERSION 3.10)
project (lmp_ppl)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

LC_DEFINE_PLUGIN (
	SRCS
		loghandler.cpp
		parser.cpp
		ppl.cpp
		tracksselectordialog.cpp
	QT_COMPONENTS Widgets
	HAS_TESTS
	)

AddLmpPplTest (parser SRCS tests/parsertest.cpp QT_COMPONENTS Concurrent)
