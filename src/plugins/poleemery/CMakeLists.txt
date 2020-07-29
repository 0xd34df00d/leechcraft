cmake_minimum_required (VERSION 3.10)
project (leechcraft_poleemery)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_search_module (Qwt REQUIRED IMPORTED_TARGET Qt5Qwt Qt5Qwt6)

LC_DEFINE_PLUGIN (
	SRCS
		poleemery.cpp
		core.cpp
		structures.cpp
		storage.cpp
		accountstab.cpp
		accountpropsdialog.cpp
		accountsmanager.cpp
		operationstab.cpp
		operationsmanager.cpp
		operationpropsdialog.cpp
		entriesmodel.cpp
		currenciesmanager.cpp
		xmlsettingsmanager.cpp
		graphstab.cpp
		graphsfactory.cpp
		entriesdelegate.cpp
	SETTINGS poleemerysettings.xml
	QT_COMPONENTS Network Sql Xml
	)
