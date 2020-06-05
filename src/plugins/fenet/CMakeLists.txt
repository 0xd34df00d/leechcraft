cmake_minimum_required (VERSION 3.3)
project (leechcraft_fenet)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		fenet.cpp
		wmfinder.cpp
		xmlsettingsmanager.cpp
		compfinder.cpp
		compparamsmanager.cpp
		compparamswidget.cpp
	SETTINGS fenetsettings.xml
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)

install (FILES leechcraft-session DESTINATION bin
	PERMISSIONS
	OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
	WORLD_EXECUTE WORLD_READ)

install (DIRECTORY share/fenet DESTINATION ${LC_SHARE_DEST}
		FILES_MATCHING PATTERN "*.sh"
		PERMISSIONS
			OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
			WORLD_EXECUTE WORLD_READ)

