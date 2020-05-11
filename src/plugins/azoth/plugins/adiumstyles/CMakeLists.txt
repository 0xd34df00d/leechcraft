cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_adiumstyles)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

LC_DEFINE_PLUGIN (azoth_adiumstyles
	SRCS
		adiumstyles.cpp
		adiumstylesource.cpp
		packproxymodel.cpp
	RESOURCES adiumstyles.qrc
	QT_COMPONENTS WebKitWidgets Xml
	TS_LANGUAGES en ru_RU
	INSTALL_SHARE
	)
