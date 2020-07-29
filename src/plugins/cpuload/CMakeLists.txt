cmake_minimum_required (VERSION 3.10)
project (leechcraft_cpuload)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		cpuload.cpp
		backend.cpp
		backendproxy.cpp
		cpuloadproxyobj.cpp
		structures.cpp
		$<$<PLATFORM_ID:Linux>:linuxbackend.cpp>
		$<$<PLATFORM_ID:FreeBSD>:freebsdbackend.cpp>
		$<$<PLATFORM_ID:Darwin>:macbackend.mm>
	QT_COMPONENTS Quick
	LINK_LIBRARIES $<$<PLATFORM_ID:FreeBSD>:kvm>
	INSTALL_SHARE
	)
