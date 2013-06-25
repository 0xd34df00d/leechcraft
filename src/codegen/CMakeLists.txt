set (SRCS
	codegen.cpp
	)

link_directories (/usr/lib/llvm)

add_executable (lc_codegen WIN32
	${SRCS}
	)
target_link_libraries (lc_codegen
	clangLex
	clangBasic
	clangAST
	clangSerialization
	clangEdit
	clangAnalysis
	clangFrontend
	clangSema
	clangDriver
	clangParse
	LLVMCore
	LLVMMC
	LLVMSupport
	)
