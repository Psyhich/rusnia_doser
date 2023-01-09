set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/")

add_executable(rusnia_doser 
	main.cpp

	${SRC}/setup.cpp
	${SRC}/solider.cpp
)
target_include_directories(doser_guns PUBLIC
	include/
)
target_link_libraries(rusnia_doser
	args-parser::args-parser
	doser_guns
)
enable_pedantic_build(rusnia_doser)
