include (CheckCXXSourceCompiles)

set (CHECK_PROGRAM "
	#include <hunspell/hunspell.hxx>
	#include <string>

	int main ()
	{
		Hunspell *hunspell = 0;
		hunspell->spell (std::string ());
	}
	")

set (CMAKE_REQUIRED_LIBRARIES ${Hunspell_LIBRARIES})

check_cxx_source_compiles ("${CHECK_PROGRAM}" HUNSPELL_HAS_CXX_OVERLOADS)

set (HUNSPELL_HAS_CXX_OVERLOADS ${Hunspell_HAS_CXX_OVERLOADS} PARENT_SCOPE)
