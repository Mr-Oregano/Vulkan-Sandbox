#include "Log.h"

namespace util {

	std::shared_ptr<spdlog::logger> Log::m_AppLogger = nullptr;
	std::shared_ptr<spdlog::logger> Log::m_VulkanLogger = nullptr;

	void Log::Init()
	{

		spdlog::set_pattern("%^[%T] %n: %v%$");

		m_AppLogger = spdlog::stdout_color_mt("APP");
		m_AppLogger->set_level(spdlog::level::trace);

		m_VulkanLogger = spdlog::stdout_color_mt("VULKAN");
		m_VulkanLogger->set_level(spdlog::level::trace);

	}
}

