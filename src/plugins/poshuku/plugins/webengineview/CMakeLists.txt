project (leechcraft_poshuku_webengineview)

LC_DEFINE_PLUGIN (
	SRCS
		cookiessyncer.cpp
		customwebpage.cpp
		customwebview.cpp
		downloaditemhandler.cpp
		icondatabase.cpp
		icondatabaseondisk.cpp
		requestinterceptor.cpp
		webengineview.cpp
	QT_COMPONENTS PrintSupport Sql WebEngineWidgets
	)
