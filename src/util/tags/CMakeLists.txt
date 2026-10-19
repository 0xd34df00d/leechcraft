add_util_library (util-tags
	SRCS
		categoryselector.cpp
		tagschange.cpp
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
	AddUtilTest (tags_change tests/tagschangetest.cpp UtilTagsChangeTest leechcraft-util-tags)
	AddUtilTest (tags_tagslineedit tests/tagslineedittest.cpp UtilTagsLineEditTest leechcraft-util-tags)
	set_tests_properties (UtilTagsLineEditTest PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif ()
