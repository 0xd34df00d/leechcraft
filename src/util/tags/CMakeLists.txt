add_util_library (util-tags
	SRCS
		categoryselector.cpp
		categoryselectorwindow.cpp
		tagschange.cpp
		tagscompletionmodel.cpp
		tagseditdelegate.cpp
		tagsfiltermodel.cpp
		tagseditoraddon.cpp
		util.cpp
	DEPENDS leechcraft-util-gui
	USES Widgets
	)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})
	AddUtilTest (tags_util tests/tagsutiltest.cpp UtilTagsUtilTest leechcraft-util-tags)
	AddUtilTest (tags_change tests/tagschangetest.cpp UtilTagsChangeTest leechcraft-util-tags)
	AddUtilTest (tags_categoryselector tests/categoryselectortest.cpp UtilTagsCategorySelectorTest leechcraft-util-tags)
	AddUtilTest (tags_tagseditoraddon tests/tagseditoraddontest.cpp UtilTagsEditorAddonTest leechcraft-util-tags)
endif ()
