#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

namespace util {

	class Log
	{

	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetAppLogger() { return m_AppLogger; }
		inline static std::shared_ptr<spdlog::logger> &GetVulkanLogger() { return m_VulkanLogger; }

	private:
		static std::shared_ptr<spdlog::logger> m_AppLogger;
		static std::shared_ptr<spdlog::logger> m_VulkanLogger;

	};

}

#define LOG_TRACE(...)			::util::Log::GetAppLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)			::util::Log::GetAppLogger()->info(__VA_ARGS__)
#define LOG_WARNING(...)		::util::Log::GetAppLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)			::util::Log::GetAppLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)		::util::Log::GetAppLogger()->critical(__VA_ARGS__)

#define LOG_VK_TRACE(...)		::util::Log::GetVulkanLogger()->trace(__VA_ARGS__)
#define LOG_VK_INFO(...)		::util::Log::GetVulkanLogger()->info(__VA_ARGS__)
#define LOG_VK_WARNING(...)		::util::Log::GetVulkanLogger()->warn(__VA_ARGS__)
#define LOG_VK_ERROR(...)		::util::Log::GetVulkanLogger()->error(__VA_ARGS__)
#define LOG_VK_CRITICAL(...)	::util::Log::GetVulkanLogger()->critical(__VA_ARGS__)