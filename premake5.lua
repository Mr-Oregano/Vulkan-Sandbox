
BINARY_DIR = "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
OBJECT_DIR = "bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

workspace "Vulkan Sandbox"
	architecture "x64"
	startproject "Vulkan Sandbox"

	configurations {
		"Debug",
		"Release"
	}

	flags "MultiProcessorCompile"

	group "Dependencies"
		include "vendor/GLFW"
	group ""

project "Vulkan Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir(BINARY_DIR)
	objdir(OBJECT_DIR)

	files {
		"src/**.h",
		"src/**.cpp"
	}

	includedirs {
		"src",
		"vendor/GLFW/include",
		"vendor/spdlog/include",
		"C:/VulkanSDK/1.1.121.0/include"
	}

	links {
		"GLFW",
		"C:/VulkanSDK/1.1.121.0/Lib/vulkan-1.lib"
	}

	defines {
		"GLFW_INCLUDE_NONE",
		"GLFW_INCLUDE_VULKAN"
	}

	filter "system:windows"
		systemversion "latest"
		defines "APP_PLATFORM_WINDOWS"
	
	filter "configurations:Debug"
		defines "APP_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "APP_RELEASE"
		runtime "Release"
		optimize "On"