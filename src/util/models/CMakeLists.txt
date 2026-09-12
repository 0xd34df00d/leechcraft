option (WITH_MODEL_SANITIZER "Enable run-time model sanitizer, linking against QtTest" OFF)

if (WITH_MODEL_SANITIZER)
	set (MODEL_SANITIZER_QT_COMPONENT Test)
endif ()

add_util_library (util-models
	SRCS
		checkableproxymodel.cpp
		fixedstringfilterproxymodel.cpp
		flatitemsmodelbase.cpp
		flattofoldersproxymodel.cpp
		mergemodel.cpp
		modeliterator.cpp
		modelitem.cpp
		modelsanitizer.cpp
		selectionproxymodel.cpp
	USES Widgets ${MODEL_SANITIZER_QT_COMPONENT}
	DEFINES $<$<BOOL:${WITH_MODEL_SANITIZER}>:WITH_MODEL_SANITIZER>
	)
