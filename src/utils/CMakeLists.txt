set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set(CONFIG_DIR_PATH "${CMAKE_SOURCE_DIR}/config/")

add_library(doser_utils STATIC
	"${SRC}/utils.cpp"
	"${SRC}/multithread.cpp"
	"${SRC}/logging.cpp"
)
target_include_directories(doser_utils
	PUBLIC
		include/
		"${CONFIG_DIR_PATH}"
)
target_link_libraries(doser_utils PUBLIC
	Threads::Threads
	spdlog::spdlog
)
enable_pedantic_build(doser_utils)
