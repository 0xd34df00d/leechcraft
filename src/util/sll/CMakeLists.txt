add_util_library (util-sll
	SRCS
		buildtagstree.cpp
		channeldevice.cpp
		domchildrenrange.cpp
		queuemanager.cpp
		regexp.cpp
		slotclosure.cpp
		urloperator.cpp
		qtutil.cpp
		urlaccessor.cpp
		raiisignalconnection.cpp
		serializejson.cpp
		xmlnode.cpp
		json.cpp
		timer.cpp
	USES Concurrent Xml
	)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})
	AddUtilTest (sll_applicative tests/applicativetest.cpp UtilSllApplicativeTest leechcraft-util-sll)
	AddUtilTest (sll_ctstring tests/ctstringtest.cpp UtilSllCtStringTest leechcraft-util-sll)
	AddUtilTest (sll_detector tests/detectortest.cpp UtilSllDetectorTest leechcraft-util-sll)

	AddUtilTest (sll_domchildrenrange tests/domchildrenrangetest.cpp UtilSllDomChildrenRangeTest leechcraft-util-sll)
	FindQtLibs(lc_util_sll_domchildrenrange_test Xml)

	AddUtilTest (sll_either tests/eithertest.cpp UtilSllEitherTest leechcraft-util-sll)
	AddUtilTest (sll_functor tests/functortest.cpp UtilSllFunctorTest leechcraft-util-sll)
	AddUtilTest (sll_monad tests/monadtest.cpp UtilSllMonadTest leechcraft-util-sll)
	AddUtilTest (sll_prelude tests/preludetest.cpp UtilSllPreludeTest leechcraft-util-sll)
	AddUtilTest (sll_qtutil tests/qtutiltest.cpp UtilSllQtUtilTest leechcraft-util-sll)
	AddUtilTest (sll_scopeguard tests/scopeguardtest.cpp UtilSllScopeGuardTest leechcraft-util-sll)
	AddUtilTest (sll_slotclosure tests/slotclosuretest.cpp UtilSllSlotClosureTest leechcraft-util-sll)
	AddUtilTest (sll_stlize tests/stlizetest.cpp UtilSllStlizeTest leechcraft-util-sll)
	AddUtilTest (sll_stringpathtrietest tests/stringpathtrietest.cpp UtilSllStringPathTrieTest leechcraft-util-sll)
	AddUtilTest (sll_tokenize tests/tokenizetest.cpp UtilSllTokenizeTest leechcraft-util-sll)
	AddUtilTest (sll_typegetter tests/typegettertest.cpp UtilSllTypeGetterTest leechcraft-util-sll)
	AddUtilTest (sll_typelist tests/typelisttest.cpp UtilSllTypelistTest leechcraft-util-sll)
	AddUtilTest (sll_views tests/viewstest.cpp UtilSllViewsTest leechcraft-util-sll)
	AddUtilTest (sll_visitor tests/visitortest.cpp UtilSllVisitorTest leechcraft-util-sll)
endif ()
