project (poshuku_filescheme)

LC_DEFINE_PLUGIN (
	SRCS filescheme.cpp schemereply.cpp
	QT_COMPONENTS Network Widgets
	RESOURCES poshukufileschemeresources.qrc
	INSTALL_SHARE
	)
