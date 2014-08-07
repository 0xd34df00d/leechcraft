set (MODELS_SRCS
	flattenfiltermodel.cpp
	flattofoldersproxymodel.cpp
	mergemodel.cpp
	modeliterator.cpp
	modelitem.cpp
	treeitem.cpp
	)

foreach (SRC ${MODELS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/models/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-models SHARED
	${MODELS_SRCS}
	)
target_link_libraries (leechcraft-util-models
	${QT_LIBRARIES}
	)
set_property (TARGET leechcraft-util-models PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-models DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-models WebKitWidgets Widgets)
