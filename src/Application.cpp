
#include "Application.h"
#include "Log.h"

QueueFamilyIndices Application::FindQueueFamilies(VkPhysicalDevice device)
{

	QueueFamilyIndices indices = { 0 };

	uint32_t propCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &propCount, nullptr);

	std::vector<VkQueueFamilyProperties> props(propCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &propCount, props.data());

	uint32_t index = 0;
	for (const auto &prop : props)
	{
		if (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.GraphicsFamily = index;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, index, this->m_Surface, &presentSupport);

		if (presentSupport)
			indices.PresentFamily = index;

		if (indices.AllAvailable())
			break;

		++index;
	}

	return indices;

}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData)
{
	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		LOG_VK_TRACE("{0}", pCallbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:	LOG_VK_INFO("{0}", pCallbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	LOG_VK_WARNING("{0}", pCallbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		LOG_VK_ERROR("{0}", pCallbackData->pMessage); break;
	}

	return VK_FALSE;

}

void SetupDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{

	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	createInfo.pfnUserCallback = &DebugCallback;
	createInfo.pUserData = nullptr;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;

}
bool Application::IsDeviceSuitable(VkPhysicalDevice device)
{

	bool RequiredQueuesAvailable = this->FindQueueFamilies(device).AllAvailable();
	bool RequiredExtensionsSupported = this->CheckDeviceExtensionSupport(device);
	bool AdequateSwapChain = false;
	if (RequiredExtensionsSupported)
	{
		SwapChainCapabilities caps = this->RetrieveSwapChainCapabilities(device);
		AdequateSwapChain = !(caps.formats.empty() || caps.presentModes.empty());
	}

	return RequiredQueuesAvailable
		&& RequiredExtensionsSupported
		&& AdequateSwapChain;

}
bool Application::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{

	uint32_t propertyCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &propertyCount, nullptr);

	std::vector<VkExtensionProperties> props(propertyCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &propertyCount, props.data());

	std::set<std::string> requestedExtensions(this->m_RequiredExtensions.begin(), this->m_RequiredExtensions.end());
	for (const auto &prop : props)
		requestedExtensions.erase(prop.extensionName);

	return requestedExtensions.empty();
}

Application::Application(bool EnableValidationLayers)
	: m_EnableValidationLayers(EnableValidationLayers)
{
}
void Application::Run()
{
	InitWindow();
	InitVulkan();
	Update();
	Shutdown();
}
void Application::InitWindow()
{
	
	if (!glfwInit())
	{
		LOG_CRITICAL("Failed to initialize the GLFW Library!");
		return;
	}

	LOG_INFO("Initialized the GLFW libary with no OpenGL!");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	this->m_Window = glfwCreateWindow(this->m_WindowWidth, this->m_WindowHeight, this->m_WindowTitle, NULL, NULL);

	if (!this->m_Window)
	{
		glfwTerminate();
		LOG_CRITICAL("Failed to create window!");
		return;
	}

}
std::vector<const char *> Application::LoadRequiredExtensions()
{

	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions = nullptr;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (this->m_EnableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;

}

void Application::InitVulkan()
{

	this->CreateVulkanInstance();
	this->CreateDebugMessenger();
	this->CreateVulkanSurface();
	this->SelectPhysicalDevice();
	this->CreateLogicalDevice();
	this->CreateSwapChain();

}

void Application::CreateVulkanInstance()
{
	if (this->m_EnableValidationLayers && !this->CheckValidationLayerSupport())
		LOG_WARNING("Validation layers not supported!");

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Testing";
	appInfo.pEngineName = "No Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;
	appInfo.pNext = nullptr;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// TODO: Read up on what exactly this means
	auto extensions = this->LoadRequiredExtensions();
	createInfo.enabledExtensionCount = (uint32_t) extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
	//

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	if (this->m_EnableValidationLayers)
	{

		LOG_INFO("Creating Debug Vulkan Instance...");
		createInfo.enabledLayerCount = (uint32_t)this->m_ValidationLayers.size();
		createInfo.ppEnabledLayerNames = this->m_ValidationLayers.data();

		SetupDebugMessengerInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;

	}
	else
	{

		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
		createInfo.pNext = nullptr;
	}

	createInfo.flags = 0;

	// Creating Vulkan Instance...
	if (vkCreateInstance(&createInfo, nullptr, &this->m_VulkanInstance) != VK_SUCCESS)
	{
		LOG_CRITICAL("Failed to create a Vulkan instance!");
		return;
	}

	LOG_INFO("Successfully created a Vulkan instance!");

	// TODO: As a challenge, try to create a function that checks if all of the extensions 
	//		 returned by glfwGetRequiredInstanceExtensions are included in the supported extensions list.

}
bool Application::CheckValidationLayerSupport()
{

	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char *layerCheck : this->m_ValidationLayers)
	{
		bool contains = false;
		for (const VkLayerProperties& availableLayer : availableLayers)
			if ((contains = strcmp(layerCheck, availableLayer.layerName) == 0))
				break;

		if (!contains)
			return false;
	}

	return true;

}

void Application::CreateDebugMessenger()
{

	if (!this->m_EnableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	SetupDebugMessengerInfo(createInfo);

	auto vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(this->m_VulkanInstance, "vkCreateDebugUtilsMessengerEXT");
	
	VkResult result = VK_SUCCESS;
	if (vkCreateDebugUtilsMessenger)
		result = vkCreateDebugUtilsMessenger(
			this->m_VulkanInstance,
			&createInfo,
			nullptr,
			&this->m_DebugMessenger);

	if (result != VK_SUCCESS)
		LOG_WARNING("Failed to create Vulkan Debug Utils Messenger");
}
void Application::DestroyDebugMessenger()
{

	auto vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(this->m_VulkanInstance, "vkDestroyDebugUtilsMessengerEXT");

	if (vkDestroyDebugUtilsMessenger)
		vkDestroyDebugUtilsMessenger(
			this->m_VulkanInstance,
			this->m_DebugMessenger,
			nullptr);

}

void Application::CreateVulkanSurface()
{
	if (glfwCreateWindowSurface(this->m_VulkanInstance, this->m_Window, nullptr, &this->m_Surface) != VK_SUCCESS)
		LOG_CRITICAL("Failed to create Win32 surface!");
}

void Application::SelectPhysicalDevice()
{

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->m_VulkanInstance, &deviceCount, nullptr);

	if (!deviceCount)
		LOG_CRITICAL("Failed to find any physical devices for vulkan on this system!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(this->m_VulkanInstance, &deviceCount, devices.data());

	for (const auto &device : devices)
	{
		if (IsDeviceSuitable(device))
		{
			VkPhysicalDeviceProperties props = {};
			vkGetPhysicalDeviceProperties(device, &props);

			LOG_INFO("Selecting Device {0} - {1}", props.deviceID, props.deviceName);
			LOG_INFO("\tDRIVER VERSION: {0}", 
				props.driverVersion);

			LOG_INFO("\tAPI VERSION: {0}.{1}.{2}",
				VK_VERSION_MAJOR(props.apiVersion),
				VK_VERSION_MINOR(props.apiVersion),
				VK_VERSION_PATCH(props.apiVersion));

			switch (props.deviceType)
			{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:	LOG_INFO("\tTYPE: Integrated GPU"); break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:		LOG_INFO("\tTYPE: Dedicated GPU"); break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:		LOG_INFO("\tTYPE: Virtual GPU"); break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:				LOG_INFO("\tTYPE: CPU"); break;
			default:										LOG_INFO("\tTYPE: UNKNOWN");
			}

			this->m_PhysicalDevice = device;
			break;
		}
	}

	if (!this->m_PhysicalDevice)
		LOG_CRITICAL("Failed to find a suitable device for vulkan!");
}

void Application::CreateLogicalDevice()
{

	QueueFamilyIndices indices = this->FindQueueFamilies(this->m_PhysicalDevice);
	float queuePriorities[] = { 1.0f };

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = queuePriorities;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.flags = 0;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = { 0 };

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = (uint32_t) queueCreateInfos.size();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	// Logical Device extensions
	createInfo.enabledExtensionCount = (uint32_t) this->m_RequiredExtensions.size();
	createInfo.ppEnabledExtensionNames = this->m_RequiredExtensions.data();
	//

	// Although ignored in recent API versions
	// still a good idea to fill out layers
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;

	if (this->m_EnableValidationLayers)
	{
		createInfo.enabledLayerCount = (uint32_t) this->m_ValidationLayers.size();
		createInfo.ppEnabledLayerNames = this->m_ValidationLayers.data();
	}

	if (vkCreateDevice(this->m_PhysicalDevice, &createInfo, nullptr, &this->m_Device) != VK_SUCCESS)
		LOG_CRITICAL("Failed to create the logical device!");

	vkGetDeviceQueue(this->m_Device, indices.GraphicsFamily.value(), 0, &this->m_GraphicsQueue);
	vkGetDeviceQueue(this->m_Device, indices.PresentFamily.value(), 0, &this->m_PresentQueue);

}

SwapChainCapabilities Application::RetrieveSwapChainCapabilities(VkPhysicalDevice device)
{
	SwapChainCapabilities caps;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->m_Surface, &caps.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->m_Surface, &formatCount, nullptr);

	if (formatCount)
	{
		caps.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->m_Surface, &formatCount, caps.formats.data());
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->m_Surface, &presentModeCount, nullptr);

	if (presentModeCount)
	{
		caps.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->m_Surface, &presentModeCount, caps.presentModes.data());
	}



	return caps;
}
VkSurfaceFormatKHR Application::SelectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats)
{
	for (const auto &surfaceFormat : surfaceFormats)
	{
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB
		&&	surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return surfaceFormat;
	}

	// TODO: Have a more advanced selection system for the desired format
	// at the moment simply return the first format given.
	//
	return surfaceFormats[0];
}
VkPresentModeKHR Application::SelectSwapChainPresentMode(const std::vector<VkPresentModeKHR> &presentModes)
{
	for (VkPresentModeKHR mode : presentModes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D Application::SelectSwapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;

	VkExtent2D extent = { (uint32_t) this->m_WindowWidth, (uint32_t) this->m_WindowHeight };

	extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
	extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));

	return extent;
}
void Application::CreateSwapChain()
{

	SwapChainCapabilities swapChainCaps = this->RetrieveSwapChainCapabilities(this->m_PhysicalDevice);

	VkSurfaceFormatKHR format = this->SelectSwapChainSurfaceFormat(swapChainCaps.formats);
	VkPresentModeKHR presentMode = this->SelectSwapChainPresentMode(swapChainCaps.presentModes);
	VkExtent2D extent = this->SelectSwapChainExtent(swapChainCaps.capabilities);

	this->m_SwapChainFormat = format.format;
	this->m_SwapChainExtent = extent;

	uint32_t desiredImageCount = swapChainCaps.capabilities.minImageCount + 1;
	if (swapChainCaps.capabilities.maxImageCount > 0
	&& desiredImageCount > swapChainCaps.capabilities.maxImageCount)
		desiredImageCount = swapChainCaps.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = this->m_Surface;

	createInfo.minImageCount = desiredImageCount;
	createInfo.imageFormat = format.format;
	createInfo.imageColorSpace = format.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = this->FindQueueFamilies(this->m_PhysicalDevice);
	uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

	if (indices.GraphicsFamily != indices.PresentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainCaps.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(this->m_Device, &createInfo, nullptr, &this->m_SwapChain) != VK_SUCCESS)
		LOG_CRITICAL("Failed to create Swap Chain!");

	uint32_t swapChainImageCount = 0;
	vkGetSwapchainImagesKHR(this->m_Device, this->m_SwapChain, &swapChainImageCount, nullptr);

	this->m_SwapChainImages.resize(swapChainImageCount);
	vkGetSwapchainImagesKHR(this->m_Device, this->m_SwapChain, &swapChainImageCount, this->m_SwapChainImages.data());

}

void Application::Update()
{

	glfwShowWindow(this->m_Window);
	while (!glfwWindowShouldClose(this->m_Window))
	{

		glfwPollEvents();

	}

}
void Application::Shutdown()
{

	vkDestroySwapchainKHR(this->m_Device, this->m_SwapChain, nullptr);
	vkDestroySurfaceKHR(this->m_VulkanInstance, this->m_Surface, nullptr);
	vkDestroyDevice(this->m_Device, nullptr);

	if (this->m_EnableValidationLayers)
		this->DestroyDebugMessenger();

	vkDestroyInstance(this->m_VulkanInstance, nullptr);

	glfwDestroyWindow(this->m_Window);
	glfwTerminate();

}

int main(void)
{

	util::Log::Init();
	LOG_INFO("Vulkan Testing");

#ifdef NDEBUG
	Application app(false);
#else
	Application app;
#endif

	app.Run();
	return 0;

}