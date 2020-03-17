#pragma once

#include <Windows.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <functional>
#include <vector>

#include <vector>
#include <optional>
#include <cstring>
#include <set>

struct QueueFamilyIndices
{
	std::optional<uint32_t> GraphicsFamily;
	std::optional<uint32_t> PresentFamily;

	inline bool AllAvailable()
	{
		return GraphicsFamily.has_value()
			&& PresentFamily.has_value();
	}
};

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

	void CreateVulkanSurface();

	void SelectPhysicalDevice();
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	bool IsDeviceSuitable(VkPhysicalDevice device);

	void CreateLogicalDevice();

	void Update();
	void Shutdown();

private:
	GLFWwindow *m_Window = nullptr;
	VkInstance m_VulkanInstance = { 0 };
	VkDebugUtilsMessengerEXT m_DebugMessenger = { 0 };
	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;

	VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
	VkQueue m_PresentQueue = VK_NULL_HANDLE;

	const std::vector<const char *> m_ValidationLayers = {

		"VK_LAYER_KHRONOS_validation"

	};

	bool m_EnableValidationLayers = true;

};