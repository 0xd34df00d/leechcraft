cmake_minimum_required (VERSION 3.10)
project (anhero)
include (InitLCPlugin NO_POLICY_SCOPE)

add_subdirectory (crashprocess)

LC_DEFINE_PLUGIN (
	SRCS anhero.cpp
	QT_COMPONENTS Gui
	INSTALL_SHARE
	)

target_compile_definitions (leechcraft_anhero PRIVATE LC_LIBSUFFIX="${LC_LIBSUFFIX}")
