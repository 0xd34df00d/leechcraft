cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_embedmedia)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

LC_DEFINE_PLUGIN (azoth_embedmedia
	SRCS embedmedia.cpp
	QT_COMPONENTS WebKitWidgets
	INSTALL_SHARE
	)
