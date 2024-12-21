set (MODELS_SRCS
	flatitemsmodelbase.cpp
	flattenfiltermodel.cpp
	flattofoldersproxymodel.cpp
	mergemodel.cpp
	modeliterator.cpp
	modelitem.cpp
	)

foreach (SRC ${MODELS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/models/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_util_library (util-models
	SOURCES ${MODELS_SRCS}
	USES Widgets
	)
