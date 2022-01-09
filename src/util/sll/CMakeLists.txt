set (SLL_SRCS
	buildtagstree.cpp
	queuemanager.cpp
	regexp.cpp
	slotclosure.cpp
	urloperator.cpp
	qtutil.cpp
	urlaccessor.cpp
	serializejson.cpp
	)

option (WITH_PCRE "Use PCRE regular expression engine for instead of Qt's one (recommended since QRegExps are slow)." ON)
set (PCRE_IMPORTED_TARGET)
if (WITH_PCRE)
	find_package (PkgConfig)
	pkg_check_modules (PCRE REQUIRED IMPORTED_TARGET libpcre)
	add_definitions (-DUSE_PCRE)
	set (PCRE_IMPORTED_TARGET PkgConfig::PCRE)
endif ()

foreach (SRC ${SLL_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/sll/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-sll${LC_LIBSUFFIX} SHARED
	${SLL_SRCS}
	)
target_link_libraries (leechcraft-util-sll${LC_LIBSUFFIX} ${PCRE_IMPORTED_TARGET})
set_property (TARGET leechcraft-util-sll${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION}.1)
install (TARGETS leechcraft-util-sll${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-sll${LC_LIBSUFFIX} Core Concurrent Xml)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})
	AddUtilTest (sll_applicative tests/applicativetest.cpp UtilSllApplicativeTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_curry tests/currytest.cpp UtilSllCurryTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_detector tests/detectortest.cpp UtilSllDetectorTest leechcraft-util-sll${LC_LIBSUFFIX})

	AddUtilTest (sll_domchildrenrange tests/domchildrenrangetest.cpp UtilSllDomChildrenRangeTest leechcraft-util-sll${LC_LIBSUFFIX})
	FindQtLibs(lc_util_sll_domchildrenrange_test Xml)

	AddUtilTest (sll_either tests/eithertest.cpp UtilSllEitherTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_functor tests/functortest.cpp UtilSllFunctorTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_monad tests/monadtest.cpp UtilSllMonadTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_monadplus tests/monadplustest.cpp UtilSllMonadPlusTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_prelude tests/preludetest.cpp UtilSllPreludeTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_scopeguard tests/scopeguardtest.cpp UtilSllScopeGuardTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_slotclosure tests/slotclosuretest.cpp UtilSllSlotClosureTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_stlize tests/stlizetest.cpp UtilSllStlizeTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_stringpathtrietest tests/stringpathtrietest.cpp UtilSllStringPathTrieTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_tokenize tests/tokenizetest.cpp UtilSllTokenizeTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_typegetter tests/typegettertest.cpp UtilSllTypeGetterTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_typelist tests/typelisttest.cpp UtilSllTypelistTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_views tests/viewstest.cpp UtilSllViewsTest leechcraft-util-sll${LC_LIBSUFFIX})
	AddUtilTest (sll_visitor tests/visitortest.cpp UtilSllVisitorTest leechcraft-util-sll${LC_LIBSUFFIX})
endif ()
