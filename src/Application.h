#pragma once

#include <Windows.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <functional>
#include <vector>

// TODO: maybe create a custom memory allocator for Vulkan.
class Application
{
public:
	Application(bool EnableValidationLayers = true);
	void Run();

private:
	void InitWindow();
	std::vector<const char*> LoadRequiredExtensions();

	void InitVulkan();
	
	void CreateVulkanInstance();
	bool CheckValidationLayerSupport();
	
	void SetupDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	VkResult CreateDebugMessenger();
	void DestroyDebugMessenger();

	void Update();
	void Shutdown();

private:
	GLFWwindow *m_Window = NULL;
	VkInstance m_VulkanInstance = { 0 };
	VkDebugUtilsMessengerEXT m_DebugMessenger = { 0 };

	std::vector<const char *> m_ValidationLayers = {

		"VK_LAYER_KHRONOS_validation"

	};

	bool m_EnableValidationLayers = true;

};