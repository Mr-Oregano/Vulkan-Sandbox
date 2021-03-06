#pragma once

#define NOMINMAX
#include <Windows.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <functional>
#include <vector>
#include <optional>
#include <set>
#include <algorithm>

#include <cstring>
#include <cstdint>
#include <fstream>

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

struct SwapChainCapabilities
{

	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

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
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	void CreateLogicalDevice();

	SwapChainCapabilities RetrieveSwapChainCapabilities(VkPhysicalDevice device);
	VkSurfaceFormatKHR SelectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats);
	VkPresentModeKHR SelectSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
	VkExtent2D SelectSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void CreateSwapChain();

	void CreateSwapChainImageViews();

	void CreateRenderPass();
	VkShaderModule Application::CreateShaderModule(const std::vector<char> &bytes);
	void CreateGraphicsPipeline();

	void CreateFramebuffers();

	void CreateCommandPool();
	void CreateCommandBuffers();

	void CreateSyncObjects();

	void Update();
	void DrawFrame();
	void Shutdown();

private:
	const char* m_WindowTitle = "Vulkan Testing";
	const int m_WindowWidth = 1280;
	const int m_WindowHeight = 720;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	size_t current_frame = 0;
	GLFWwindow *m_Window = nullptr;
	
	VkInstance m_VulkanInstance = { 0 };
	VkDebugUtilsMessengerEXT m_DebugMessenger = { 0 };
	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;

	VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
	VkQueue m_PresentQueue = VK_NULL_HANDLE;

	VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainFormat;
	VkExtent2D m_SwapChainExtent = { 0 };

	std::vector<VkImageView> m_SwapChainImageViews;

	VkRenderPass m_RenderPass;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;

	std::vector<VkFramebuffer> m_SwapChainFramebuffers;

	VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinshedSemaphores;
	std::vector<VkFence> m_InFlightFences;
	std::vector<VkFence> m_ImagesInFlight;

	const std::vector<const char *> m_ValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char *> m_RequiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	bool m_EnableValidationLayers = true;

};