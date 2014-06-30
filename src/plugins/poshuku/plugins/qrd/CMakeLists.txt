include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (QRD_SRCS
	qrd.cpp
	)
set (QRD_RESOURCES
	)
CreateTrs ("poshuku_qrd" "en;ru_RU" QRD_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_qrd" "en;ru_RU" "${QRD_SRCS}" "" "")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (QREncode REQUIRED)

include_directories (${QRENCODE_INCLUDE_DIR})

QT4_ADD_RESOURCES (QRD_RCCS ${QRD_RESOURCES})
add_library (leechcraft_poshuku_qrd SHARED
	${QRD_COMPILED_TRANSLATIONS}
	${QRD_SRCS}
	${QRD_MOC_SRCS}
	${QRD_RCCS}
	)
target_link_libraries (leechcraft_poshuku_qrd
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QRENCODE_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_qrd DESTINATION ${LC_PLUGINS_DEST})
#install (FILES poshukuqrdsettings.xml DESTINATION ${LC_SETTINGS_DEST})
