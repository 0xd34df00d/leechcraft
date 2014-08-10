include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	)
set (SRCS
	simplestorage.cpp
	)

set (SIMPLESTORAGE_RESOURCES simplestorageresources.qrc)
QtAddResources (SIMPLESTORAGE_RCCS ${SIMPLESTORAGE_RESOURCES})

add_library (leechcraft_secman_simplestorage SHARED
	${SIMPLESTORAGE_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_secman_simplestorage
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_secman_simplestorage DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_secman_simplestorage Core)
