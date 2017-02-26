if (NOT LEECHCRAFT_LIBRARIES)
	cmake_policy (SET CMP0003 NEW)

	set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake;${CMAKE_MODULE_PATH}")
	set (CMAKE_MODULE_PATH "/usr/local/share/apps/cmake/modules;/usr/share/apps/cmake/modules;${CMAKE_MODULE_PATH}")

	find_package (Boost REQUIRED)
	find_package (LeechCraft-qt5 REQUIRED)
endif ()
