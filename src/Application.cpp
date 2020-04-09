
#include "Application.h"
#include "Log.h"

static std::vector<char> ReadFile(const std::string &path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		LOG_ERROR("Failed to load the file: {0}", path);
		return std::vector<char>();
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);
	
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;

}

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
	this->CreateSwapChainImageViews();
	this->CreateRenderPass();
	this->CreateGraphicsPipeline();
	this->CreateFramebuffers();
	this->CreateCommandPool();
	this->CreateCommandBuffers();
	this->CreateSyncObjects();

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
		exit(-1);
	}

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
	{
		LOG_WARNING("Failed to create Vulkan Debug Utils Messenger");
		exit(-1);
	}
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
	{
		LOG_CRITICAL("Failed to create Win32 surface!");
		exit(-1);
	}
}

void Application::SelectPhysicalDevice()
{

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->m_VulkanInstance, &deviceCount, nullptr);

	if (!deviceCount)
	{
		LOG_CRITICAL("Failed to find any physical devices for vulkan on this system!");
		exit(-1);
	}

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
	{
		LOG_CRITICAL("Failed to find a suitable device for vulkan!");
		exit(-1);
	}
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
	{
		LOG_CRITICAL("Failed to create the logical device!");
		exit(-1);
	}

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
	{
		LOG_CRITICAL("Failed to create Swap Chain!");
		exit(-1);
	}

	uint32_t swapChainImageCount = 0;
	vkGetSwapchainImagesKHR(this->m_Device, this->m_SwapChain, &swapChainImageCount, nullptr);

	this->m_SwapChainImages.resize(swapChainImageCount);
	vkGetSwapchainImagesKHR(this->m_Device, this->m_SwapChain, &swapChainImageCount, this->m_SwapChainImages.data());

}

void Application::CreateSwapChainImageViews()
{

	this->m_SwapChainImageViews.resize(this->m_SwapChainImages.size());

	for (int i = 0; i < this->m_SwapChainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = this->m_SwapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = this->m_SwapChainFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(this->m_Device, &createInfo, nullptr, &this->m_SwapChainImageViews[i]) != VK_SUCCESS)
		{
			LOG_CRITICAL("Failed to create Swap Chain image views!");
			exit(-1);
		}
	}
}

void Application::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = this->m_SwapChainFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(this->m_Device, &renderPassCreateInfo, nullptr, &this->m_RenderPass) != VK_SUCCESS)
	{
		LOG_CRITICAL("Failed to create render pass!");
		exit(-1);
	}
}

VkShaderModule Application::CreateShaderModule(const std::vector<char>& bytes)
{

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = bytes.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes.data());

	VkShaderModule module = nullptr;
	if (vkCreateShaderModule(this->m_Device, &createInfo, nullptr, &module) != VK_SUCCESS)
	{
		LOG_ERROR("Failed to create shader module!");
		return nullptr;
	}

	return module;

}
void Application::CreateGraphicsPipeline()
{
	auto vertexShaderBytes = ReadFile("assets/shaders/vertex.spv");
	auto fragmentShaderBytes = ReadFile("assets/shaders/fragment.spv");

	VkShaderModule vertexModule = CreateShaderModule(vertexShaderBytes);
	VkShaderModule fragmentModule = CreateShaderModule(fragmentShaderBytes);

	VkPipelineShaderStageCreateInfo vertexStageCreateInfo = {};
	vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexStageCreateInfo.module = vertexModule;
	vertexStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentStageCreateInfo = {};
	fragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStageCreateInfo.module = fragmentModule;
	fragmentStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexStageCreateInfo, fragmentStageCreateInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float) this->m_SwapChainExtent.width;
	viewport.height = (float) this->m_SwapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = this->m_SwapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
										  VK_COLOR_COMPONENT_G_BIT |
										  VK_COLOR_COMPONENT_B_BIT | 
										  VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;

	if (vkCreatePipelineLayout(this->m_Device, &pipelineLayoutCreateInfo, nullptr, &this->m_PipelineLayout) != VK_SUCCESS)
	{
		LOG_ERROR("Failed to create pipeline layout!");
		exit(-1);
	}

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;

	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.pDynamicState = nullptr;

	pipelineCreateInfo.layout = this->m_PipelineLayout;

	pipelineCreateInfo.renderPass = this->m_RenderPass;
	pipelineCreateInfo.subpass = 0;

	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(this->m_Device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &this->m_GraphicsPipeline) != VK_SUCCESS)
	{
		LOG_CRITICAL("Failed to create graphics pipeline!");
		exit(-1);
	}

	vkDestroyShaderModule(this->m_Device, vertexModule, nullptr);
	vkDestroyShaderModule(this->m_Device, fragmentModule, nullptr);

}

void Application::CreateFramebuffers()
{

	this->m_SwapChainFramebuffers.resize(this->m_SwapChainImageViews.size());

	for (int i = 0; i < this->m_SwapChainImageViews.size(); ++i)
	{
		VkImageView attachments[] = {
			this->m_SwapChainImageViews[i]
		};

		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = this->m_RenderPass;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = attachments;
		createInfo.width = this->m_SwapChainExtent.width;
		createInfo.height = this->m_SwapChainExtent.height;
		createInfo.layers = 1;

		if (vkCreateFramebuffer(this->m_Device, &createInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
		{
			LOG_CRITICAL("Failed to create framebuffer!");
			exit(-1);
		}
	}
}

void Application::CreateCommandPool()
{

	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(this->m_PhysicalDevice);

	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();
	poolCreateInfo.flags = 0;

	if (vkCreateCommandPool(this->m_Device, &poolCreateInfo, nullptr, &this->m_CommandPool) != VK_SUCCESS)
	{
		LOG_CRITICAL("Failed to create command pool!");
		exit(-1);
	}

}
void Application::CreateCommandBuffers()
{

	this->m_CommandBuffers.resize(this->m_SwapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = (uint32_t) this->m_SwapChainFramebuffers.size();
	allocInfo.commandPool = this->m_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkAllocateCommandBuffers(this->m_Device, &allocInfo, this->m_CommandBuffers.data()))
	{
		LOG_CRITICAL("Failed to create command buffers");
		exit(-1);
	}

	for (size_t i = 0; i < this->m_CommandBuffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(this->m_CommandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			LOG_CRITICAL("Failed to begin recording command buffer!");
			exit(-1);
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = this->m_RenderPass;
		renderPassInfo.framebuffer = this->m_SwapChainFramebuffers[i];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = this->m_SwapChainExtent;

		VkClearValue clearColor = { 0.015f, 0.015f, 0.02f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(this->m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(this->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->m_GraphicsPipeline);
		vkCmdDraw(this->m_CommandBuffers[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(this->m_CommandBuffers[i]);

		if (vkEndCommandBuffer(this->m_CommandBuffers[i]) != VK_SUCCESS)
		{
			LOG_CRITICAL("Failed to complete the the command buffer!");
			exit(-1);
		}
	}
}

void Application::CreateSyncObjects()
{

	this->m_ImageAvailableSemaphores.resize(this->MAX_FRAMES_IN_FLIGHT);
	this->m_RenderFinshedSemaphores.resize(this->MAX_FRAMES_IN_FLIGHT);
	this->m_InFlightFences.resize(this->MAX_FRAMES_IN_FLIGHT);
	this->m_ImagesInFlight.resize(this->m_SwapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < this->MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (vkCreateSemaphore(this->m_Device, &semaphoreCreateInfo, nullptr, &this->m_ImageAvailableSemaphores[i]) != VK_SUCCESS
		|| vkCreateSemaphore(this->m_Device, &semaphoreCreateInfo, nullptr, &this->m_RenderFinshedSemaphores[i]) != VK_SUCCESS
		|| vkCreateFence(this->m_Device, &fenceCreateInfo, nullptr, &this->m_InFlightFences[i]) != VK_SUCCESS)
		{
			LOG_CRITICAL("Failed to create sync objects!");
			exit(-1);
		}
	}
}

bool IsEqual(int x)
{

	int y = 0;
	return x == y;

}

void Application::Update()
{

	glfwShowWindow(this->m_Window);
	while (!glfwWindowShouldClose(this->m_Window))
	{
		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(this->m_Device);

}
void Application::DrawFrame()
{
	vkWaitForFences(this->m_Device, 1, &this->m_InFlightFences[current_frame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR(this->m_Device, this->m_SwapChain, UINT64_MAX, this->m_ImageAvailableSemaphores[current_frame], VK_NULL_HANDLE, &imageIndex);

	if (this->m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(this->m_Device, 1, &this->m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

	this->m_ImagesInFlight[imageIndex] = this->m_InFlightFences[current_frame];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { this->m_ImageAvailableSemaphores[current_frame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->m_CommandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { this->m_RenderFinshedSemaphores[current_frame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(this->m_Device, 1, &this->m_InFlightFences[current_frame]);
	if (vkQueueSubmit(this->m_GraphicsQueue, 1, &submitInfo, this->m_InFlightFences[current_frame]) != VK_SUCCESS)
	{
		LOG_CRITICAL("Failed to submit draw buffer command!");
		exit(-1);
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { this->m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(this->m_GraphicsQueue, &presentInfo);

	current_frame = (current_frame + 1) % this->MAX_FRAMES_IN_FLIGHT;

}
void Application::Shutdown()
{

	for (int i = 0; i < this->MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(this->m_Device, this->m_RenderFinshedSemaphores[i], nullptr);
		vkDestroySemaphore(this->m_Device, this->m_ImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(this->m_Device, this->m_InFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(this->m_Device, this->m_CommandPool, nullptr);

	for (VkFramebuffer framebuffer : this->m_SwapChainFramebuffers)
		vkDestroyFramebuffer(this->m_Device, framebuffer, nullptr);

	vkDestroyPipeline(this->m_Device, this->m_GraphicsPipeline, nullptr);

	vkDestroyPipelineLayout(this->m_Device, this->m_PipelineLayout, nullptr);
	vkDestroyRenderPass(this->m_Device, this->m_RenderPass, nullptr);

	for (VkImageView view : this->m_SwapChainImageViews)
		vkDestroyImageView(this->m_Device, view, nullptr);

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