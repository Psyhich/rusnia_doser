set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set(CONFIG_DIR_PATH "${CMAKE_SOURCE_DIR}/config/")

option(ENFORCE_DEBUG_LOGGING "Force debug logs" FALSE)

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
target_link_libraries(doser_utils
	PUBLIC
		Threads::Threads
		spdlog::spdlog
)
target_compile_definitions(doser_utils PUBLIC
	$<$<OR:$<CONFIG:Debug>,$<BOOL:${ENFORCE_DEBUG_LOGGING}>>:DEBUG_LOGS>
)
target_precompile_headers(doser_utils
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include/logging.h"
)

enable_pedantic_build(doser_utils)
