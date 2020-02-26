#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

namespace util {

	class Log
	{

	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return logger; }

	private:
		static std::shared_ptr<spdlog::logger> logger;

	};

}

#define LOG_TRACE(...) ::util::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) ::util::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARNING(...) ::util::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::util::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::util::Log::GetLogger()->critical(__VA_ARGS__)