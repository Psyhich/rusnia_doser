set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/")

add_library(doser_guns STATIC
	${SRC}/http_gun.cpp
	${SRC}/tcp_gun.cpp
	${SRC}/udp_gun.cpp
	${SRC}/gun_holder.cpp
)
target_include_directories(doser_guns
	PUBLIC include/
)
target_link_libraries(doser_guns PUBLIC
	doser_network
)
enable_pedantic_build(doser_guns)
