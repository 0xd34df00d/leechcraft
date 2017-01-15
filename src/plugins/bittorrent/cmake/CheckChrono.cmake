include (CheckCXXSourceCompiles)

set (CHECK_PROGRAM "
	#include <libtorrent/session.hpp>

	int main ()
	{
		libtorrent::session sess;
		sess.wait_for_alert (libtorrent::time_duration {});
	}
	")

set (CMAKE_REQUIRED_LIBRARIES ${Boost_SYSTEM_LIBRARY} ${RBTorrent_LIBRARY})

set (CMAKE_REQUIRED_DEFINITIONS "-DBOOST_ASIO_HAS_STD_CHRONO ${CMAKE_CXX11_STANDARD_COMPILE_OPTION}")
check_cxx_source_compiles ("${CHECK_PROGRAM}" LIBTORRENT_BUILT_WITH_STD_CHRONO)

set (CMAKE_REQUIRED_DEFINITIONS "-DBOOST_ASIO_DISABLE_STD_CHRONO ${CMAKE_CXX11_STANDARD_COMPILE_OPTION}")
check_cxx_source_compiles ("${CHECK_PROGRAM}" LIBTORRENT_BUILT_WITH_BOOST_CHRONO)
