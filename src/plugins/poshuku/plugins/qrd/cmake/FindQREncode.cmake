find_path (QRENCODE_INCLUDE_DIR qrencode.h
	PATH_SUFFIXES include/qrencode include
	PATHS
		/usr/local
		/usr
		/opt/local
		/opt
	)

find_library (QRENCODE_LIBRARIES
	NAMES qrencode
	)

if (QRENCODE_LIBRARIES AND QRENCODE_INCLUDE_DIR)
	set (QRENCODE_FOUND TRUE)
endif (QRENCODE_LIBRARIES AND QRENCODE_INCLUDE_DIR)
