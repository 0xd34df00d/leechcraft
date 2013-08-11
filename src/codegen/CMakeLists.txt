set (SRCS
	codegen.cpp
	)

link_directories (/usr/lib/llvm)

add_executable (lc_codegen WIN32
	${SRCS}
	)
