cmake_minimum_required (VERSION 2.8)
project (leechcraft_fenet)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	fenet.cpp
	wmfinder.cpp
	xmlsettingsmanager.cpp
	compfinder.cpp
	compparamsmanager.cpp
	compparamswidget.cpp
	)
CreateTrs("fenet" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("fenet" "en;ru_RU" "${SRCS}" "${FORMS}" "fenetsettings.xml")

add_library (leechcraft_fenet SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_fenet
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_fenet DESTINATION ${LC_PLUGINS_DEST})
install (FILES leechcraft-session DESTINATION bin
		PERMISSIONS
			OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
			WORLD_EXECUTE WORLD_READ)
install (FILES fenetsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/xsessions DESTINATION share)
install (DIRECTORY share/fenet DESTINATION ${LC_SHARE_DEST} FILES_MATCHING PATTERN "*.json")
install (DIRECTORY share/fenet DESTINATION ${LC_SHARE_DEST}
		FILES_MATCHING PATTERN "*.sh"
		PERMISSIONS
			OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
			WORLD_EXECUTE WORLD_READ)

FindQtLibs (leechcraft_fenet Widgets)
