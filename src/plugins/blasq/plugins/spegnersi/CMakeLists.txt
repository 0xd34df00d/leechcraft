cmake_minimum_required (VERSION 3.3)
project (leechcraft_blasq_spegnersi)
include (InitLCPlugin OPTIONAL)

find_package (QtKOAuth REQUIRED)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QTKOAUTH_INCLUDE_DIR}
	)
set (SPEGNERSI_SRCS
	spegnersi.cpp
	flickrservice.cpp
	flickraccount.cpp
	)
set (SPEGNERSI_RESOURCES
	blasqspegnersiresources.qrc
	)
CreateTrs("blasq_spegnersi" "en;ru_RU" SPEGNERSI_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("blasq_spegnersi" "en;ru_RU" "${SPEGNERSI_SRCS}" "${SPEGNERSI_FORMS}" "")

QtAddResources (SPEGNERSI_RCCS ${SPEGNERSI_RESOURCES})

add_library (leechcraft_blasq_spegnersi SHARED
	${SPEGNERSI_COMPILED_TRANSLATIONS}
	${SPEGNERSI_SRCS}
	${SPEGNERSI_RCCS}
	)
target_link_libraries (leechcraft_blasq_spegnersi
	${LEECHCRAFT_LIBRARIES}
	${QTKOAUTH_LIBRARIES}
	)
install (TARGETS leechcraft_blasq_spegnersi DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_blasq_spegnersi Gui Network Xml)
