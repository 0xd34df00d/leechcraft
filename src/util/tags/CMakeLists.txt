add_util_library (util-tags
	SRCS
		categoryselector.cpp
		tagscompleter.cpp
		tagscompletionmodel.cpp
		tagseditdelegate.cpp
		tagslineedit.cpp
		tagsfiltermodel.cpp
		util.cpp
	DEPENDS leechcraft-util-gui
	USES Widgets
	)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})
	AddUtilTest (tags_util tests/tagsutiltest.cpp UtilTagsUtilTest leechcraft-util-tags)
endif ()
