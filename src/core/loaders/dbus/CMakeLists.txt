set (SRCS
	pluginwrapper.cpp
	server.cpp
	marshalling.cpp
	infoserverwrapper.cpp
	coreproxyserverwrapper.cpp
	coreproxyproxy.cpp
	../ipluginloader.cpp
	../sopluginloader.cpp
	)

add_executable (lc_plugin_wrapper WIN32
	${SRCS}
	)
target_link_libraries (lc_plugin_wrapper
		${Boost_PROGRAM_OPTIONS_LIBRARY}
		${LEECHCRAFT_LIBRARIES}
		${ADDITIONAL_LIBS}
		)
install (TARGETS lc_plugin_wrapper DESTINATION ${LC_BINDIR})

FindQtLibs (lc_plugin_wrapper DBus)
