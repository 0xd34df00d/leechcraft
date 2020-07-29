cmake_minimum_required (VERSION 3.10)
project (leechcraft_musiczombie)
include (InitLCPlugin NO_POLICY_SCOPE)
include (CMakeDependentOption)

option (WITH_MUSICZOMBIE_CHROMAPRINT "Build support for Chromaprints in MusicZombie" ON)

if (WITH_MUSICZOMBIE_CHROMAPRINT)
	find_package (PkgConfig)
	pkg_check_modules (MzChroma REQUIRED IMPORTED_TARGET libchromaprint libavcodec libavformat libavresample libavutil)
	set (CHROMA_LIBRARIES PkgConfig::MzChroma)
	add_definitions (-DWITH_CHROMAPRINT)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		musiczombie.cpp
		pendingdisco.cpp
		util.cpp
		$<$<BOOL:${WITH_MUSICZOMBIE_CHROMAPRINT}>:
			chroma.cpp
			pendingtagsfetch.cpp
		>
	QT_COMPONENTS Gui Network Xml
	LINK_LIBRARIES ${CHROMA_LIBRARIES}
	)
