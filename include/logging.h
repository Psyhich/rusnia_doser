#ifndef LOGGING_H
#define LOGGING_H

#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> consoleSink;
static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSink;

bool SetupLogging();

#endif // LOGGING_H
