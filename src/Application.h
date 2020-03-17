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
	
	void CreateDebugMessenger();
	void DestroyDebugMessenger();

	void SelectPhysicalDevice();

	void CreateLogicalDevice();

	void Update();
	void Shutdown();

private:
	GLFWwindow *m_Window = nullptr;
	VkInstance m_VulkanInstance = { 0 };
	VkDebugUtilsMessengerEXT m_DebugMessenger = { 0 };

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;

	VkQueue m_GraphicsQueue = VK_NULL_HANDLE;

	const std::vector<const char *> m_ValidationLayers = {

		"VK_LAYER_KHRONOS_validation"

	};

	bool m_EnableValidationLayers = true;

};