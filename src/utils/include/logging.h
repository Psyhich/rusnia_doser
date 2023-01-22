#ifndef LOGGING_H
#define LOGGING_H

#include <memory>

#ifdef DEBUG_LOGS
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#else
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_ERROR
#endif // DEBUG_LOGS

#include "spdlog/spdlog.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace Setup
{
	static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> consoleSink;
	static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSink;

	bool SetupLogging();
}

#endif // LOGGING_H
