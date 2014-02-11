set (SRCS
	codegen.cpp
	)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (LLVM)

add_definitions (${LLVM_COMPILE_FLAGS})

include_directories (${LLVM_INCLUDE_DIR})

add_executable (lc_codegen WIN32
	${SRCS}
	)

target_link_libraries (lc_codegen ${LLVM_LDFLAGS})
