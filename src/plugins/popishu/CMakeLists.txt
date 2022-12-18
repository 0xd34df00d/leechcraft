cmake_minimum_required (VERSION 3.10)
project (popishu)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package (QScintilla REQUIRED)

LC_DEFINE_PLUGIN (
	SRCS
		popishu.cpp
		editorpage.cpp
		xmlsettingsmanager.cpp
		replacedialog.cpp
	SETTINGS popishusettings.xml
	QT_COMPONENTS Widgets
	LINK_LIBRARIES QSCINTILLA::QSCINTILLA
	INSTALL_SHARE
	)
