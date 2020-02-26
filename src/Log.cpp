#include "Log.h"

namespace util {

	std::shared_ptr<spdlog::logger> Log::logger = nullptr;

	void Log::Init()
	{

		spdlog::set_pattern("%^[%T] %n: %v%$");
		logger = spdlog::stdout_color_mt("APP");
		logger->set_level(spdlog::level::trace);

	}
}

