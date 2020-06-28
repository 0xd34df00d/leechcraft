project (leechcraft_poshuku_onlinebookmarks_readitlater)

LC_DEFINE_PLUGIN (
	SRCS
		readitlater.cpp
		readitlaterservice.cpp
		readitlaterauthwidget.cpp
		readitlateraccount.cpp
		readitlaterapi.cpp
	RESOURCES readitlaterresources.qrc
	QT_COMPONENTS Network Widgets
	INSTALL_SHARE
	)
