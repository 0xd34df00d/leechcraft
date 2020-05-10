cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_depester)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

LC_DEFINE_PLUGIN (azoth_depester
	SRCS
		depester.cpp
	RESOURCES depesterresources.qrc
	QT_COMPONENTS Core Widgets
	TS_LANGUAGES en es ru_RU uk_UA
	)
