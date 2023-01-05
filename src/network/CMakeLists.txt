set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/")

add_library(doser_network STATIC
	${SRC}/net_utils.cpp
	${SRC}/proxy_checker.cpp
	${SRC}/uri.cpp

	${SRC}/wrappers/http_wrapper.cpp
	${SRC}/wrappers/tcp_wrapper.cpp
	${SRC}/wrappers/udp_wrapper.cpp
)
target_include_directories(doser_network
	PUBLIC include/
	PRIVATE include/wrappers/
)
target_link_libraries(doser_network
	PUBLIC doser_utils
)
enable_pedantic_build(doser_network)
