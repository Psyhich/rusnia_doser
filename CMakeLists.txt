cmake_minimum_required(VERSION 3.8)

project(rusnia_doser VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(BUILD_SHARED_LIBS False)

set(SOURCES
	main.cpp

	src/solider.cpp
	src/api_interface.cpp

	src/http_gun.cpp
	src/tcp_gun.cpp

	src/curl_wrapper.cpp
	src/tcp_wrapper.cpp

	src/utils.cpp
	src/logging.cpp
	src/uri.cpp
	src/net_utils.cpp
)

add_subdirectory(lib/curl)
add_subdirectory(lib/openssl-cmake)
add_subdirectory(lib/json)
add_subdirectory(lib/spdlog)

find_package(Threads REQUIRED)

add_executable(rusnia_doser ${SOURCES})

target_link_libraries(rusnia_doser
	Threads::Threads
	ssl
	libcurl
	spdlog
	nlohmann_json::nlohmann_json
)


target_include_directories(rusnia_doser PUBLIC 
	${CURL_INCLUDE_DIRS}
	include/
	config/
	lib/args-parser/
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
