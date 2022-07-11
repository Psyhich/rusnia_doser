#include <iostream>
#include <memory>

#include "logging.h"
#include "spdlog/common.h"

using namespace Setup;

bool Setup::SetupLogging()
{
	try
	{
		consoleSink = 
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_level(spdlog::level::warn);
		consoleSink->set_pattern("[ %D %T:%e ] [ %^%l%$ ] -> %v");

		fileSink = 
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.log", true);
		fileSink->set_level(spdlog::level::info);
		fileSink->set_pattern("[ %t ] [ %D %T:%e ] [ %l ] %s:%# -> %v");
		
		spdlog::sinks_init_list sinks;

		spdlog::set_default_logger(
			std::make_shared<spdlog::logger>("main_logger", 
				spdlog::sinks_init_list{consoleSink, fileSink}));
	}
	catch(const spdlog::spdlog_ex &exception)
	{
		std::cerr << "Failed to setup logging!\n Reason:" << std::endl;
		std::cerr << exception.what() << std::endl;
	}
	return true;
}
