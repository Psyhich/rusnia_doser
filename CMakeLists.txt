cmake_minimum_required(VERSION 3.8)

project(rusnia_doser VERSION 0.1 LANGUAGES CXX)


set(BUILD_SHARED_LIBS False)

set(HEADERS
	config/config.h
	include/attacker.h
	include/curl_wrapper.h
	include/utils.h
	include/response.hpp
	include/resource_checker.h
	include/proxy_manager.h
)

set(SOURCES
	main.cpp
	src/attacker.cpp
	src/curl_wrapper.cpp
	src/utils.cpp
	src/resource_checker.cpp
	src/proxy_manager.cpp
)

add_subdirectory(lib/curl)
add_subdirectory(lib/openssl-cmake)
add_subdirectory(lib/json)

find_package(Threads REQUIRED)

add_executable(rusnia_doser ${HEADERS} ${SOURCES})


target_link_libraries(rusnia_doser
	Threads::Threads
	ssl
	libcurl
	nlohmann_json::nlohmann_json
)


target_include_directories(rusnia_doser PRIVATE 
	${CURL_INCLUDE_DIRS}
	include/
	config/
)

set_property(TARGET rusnia_doser PROPERTY 
	CXX_STANDARD 17)
set_property(TARGET rusnia_doser PROPERTY 
	CXX_STANDARD_REQUIRED ON)
set_property(TARGET rusnia_doser PROPERTY 
	CXX_COMPILE_FEATURES ON)

# For clangd analyzer
if(CMAKE_BUILD_TYPE MATCHES Debug)
	set_target_properties(${PROJECT_NAME} 
		PROPERTIES EXPORT_COMPILE_COMMANDS true)

	execute_process(
		COMMAND ${CMAKE_COMMAND} -E create_symlink
			${CMAKE_BINARY_DIR}/compile_commands.json
			${CMAKE_SOURCE_DIR}/compile_commands.json
	)
endif()
