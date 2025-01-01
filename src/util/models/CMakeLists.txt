set (MODELS_SRCS
	flatitemsmodelbase.cpp
	flattenfiltermodel.cpp
	flattofoldersproxymodel.cpp
	mergemodel.cpp
	modeliterator.cpp
	modelitem.cpp
	)

add_util_library (util-models
	SOURCES ${MODELS_SRCS}
	USES Widgets
	)
