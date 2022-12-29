cmake_minimum_required(VERSION 3.20)

project(rusnia_doser LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/")

find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(CURL REQUIRED)
find_package(args-parser REQUIRED)

add_executable(rusnia_doser 
	main.cpp

	src/logging.cpp
	src/setup.cpp

	src/solider.cpp

	src/http_gun.cpp
	src/tcp_gun.cpp
	src/udp_gun.cpp

	src/curl_wrapper.cpp
	src/tcp_wrapper.cpp
	src/udp_wrapper.cpp

	src/utils.cpp
	src/net_utils.cpp
	src/proxy_checker.cpp
	src/uri.cpp
	src/multithread.cpp
)

target_link_libraries(rusnia_doser
	Threads::Threads
	spdlog::spdlog
	nlohmann_json::nlohmann_json
	CURL::libcurl
	args-parser::args-parser
)

target_include_directories(rusnia_doser PUBLIC 
	include/
	config/
)

target_compile_options(rusnia_doser PRIVATE
	"-Wall" "-Wextra" "-Wpedantic" "-Werror"
)

set_target_properties(rusnia_doser PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_COMPILE_FEATURES ON)

# For clangd analyzer
if(CMAKE_BUILD_TYPE MATCHES Debug)
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E create_symlink
			${CMAKE_BINARY_DIR}/compile_commands.json
			${CMAKE_SOURCE_DIR}/compile_commands.json
	)
endif()

install(TARGETS rusnia_doser)
