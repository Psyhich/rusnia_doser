cmake_minimum_required(VERSION 3.20)

project(rusnia_doser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/")

find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
find_package(CURL REQUIRED)
find_package(args-parser REQUIRED)

function(enable_pedantic_build TARGET)
	target_compile_options("${TARGET}" PRIVATE
		-Wall -Wextra -Wpedantic -Werror
	)
endfunction()

add_subdirectory(src/utils)
add_subdirectory(src/network)
add_subdirectory(src/guns)
add_subdirectory(src/doser)

# For clangd analyzer
if(CMAKE_BUILD_TYPE MATCHES Debug)
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E create_symlink
			${CMAKE_BINARY_DIR}/compile_commands.json
			${CMAKE_SOURCE_DIR}/compile_commands.json
	)
endif()

install(TARGETS rusnia_doser)
