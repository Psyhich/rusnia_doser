cmake_minimum_required(VERSION 3.20)

project(rusnia_doser LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(BUILD_SHARED_LIBS FALSE)

set(SOURCES
	main.cpp

	src/logging.cpp
	src/setup.cpp

	src/solider.cpp
	src/api_interface.cpp

	src/http_gun.cpp
	src/tcp_gun.cpp

	src/curl_wrapper.cpp
	src/tcp_wrapper.cpp

	src/dynamic_target.cpp

	src/utils.cpp
	src/net_utils.cpp

	src/uri.cpp

	src/multithread.cpp
)

add_subdirectory(lib/json)
add_subdirectory(lib/spdlog)

# Including conan
include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
conan_basic_setup()

find_package(Threads REQUIRED)

add_executable(rusnia_doser ${SOURCES})

target_link_libraries(rusnia_doser
	Threads::Threads
	spdlog
	nlohmann_json::nlohmann_json
	${CONAN_LIBS}
)

target_include_directories(rusnia_doser PUBLIC 
	${CURL_INCLUDE_DIRS}
	include/
	config/
	lib/args-parser/
)

target_compile_options(rusnia_doser PRIVATE
	"-Wall" "-Wextra" "-Wpedantic" "-Werror"
)

set_property(TARGET rusnia_doser PROPERTY 
	CXX_STANDARD 17)
set_property(TARGET rusnia_doser PROPERTY 
	CXX_STANDARD_REQUIRED ON)
set_property(TARGET rusnia_doser PROPERTY 
	CXX_COMPILE_FEATURES ON)

# For clangd analyzer
if(CMAKE_BUILD_TYPE MATCHES Debug)
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E create_symlink
			${CMAKE_BINARY_DIR}/compile_commands.json
			${CMAKE_SOURCE_DIR}/compile_commands.json
	)
endif()
