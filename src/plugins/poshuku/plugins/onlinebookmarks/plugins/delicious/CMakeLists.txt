project (poshuku_onlinebookmarks_delicious)

LC_DEFINE_PLUGIN (
	SRCS
		delicious.cpp
		deliciousauthwidget.cpp
		deliciousservice.cpp
		deliciousaccount.cpp
		deliciousapi.cpp
	RESOURCES deliciousresources.qrc
	QT_COMPONENTS Network Widgets
	INSTALL_SHARE
	)
