
#include "Application.h"
#include "Log.h"

#include <vector>
#include <optional>
#include <cstring>	

struct QueueFamilyIndices
{
	std::optional<uint32_t> GraphicsFamily;
	inline bool AllAvailable() { return GraphicsFamily.has_value(); }
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
{

	QueueFamilyIndices indices = { 0 };

	uint32_t propCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &propCount, nullptr);

	std::vector<VkQueueFamilyProperties> props(propCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &propCount, props.data());

	int index = 0;
	for (const auto &prop : props)
	{
		if (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.GraphicsFamily = index;

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
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:	LOG_VK_TRACE("{0}", pCallbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		LOG_VK_INFO("{0}", pCallbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	LOG_VK_WARNING("{0}", pCallbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		LOG_VK_ERROR("{0}", pCallbackData->pMessage); __debugbreak(); break;
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
bool IsDeviceSuitable(VkPhysicalDevice device)
{

	return FindQueueFamilies(device).AllAvailable();

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

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (!glfwInit())
	{
		LOG_CRITICAL("Failed to initialize the GLFW Library!");
		return;
	}

	LOG_INFO("Initialized the GLFW libary with no OpenGL!");

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	this->m_Window = glfwCreateWindow(1280, 720, "Vulkan Testing", NULL, NULL);

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
	this->SelectPhysicalDevice();

}

void Application::CreateVulkanInstance()
{
	if (this->m_EnableValidationLayers && !this->CheckValidationLayerSupport())
		LOG_WARNING("Validation layers not supported!");

	VkApplicationInfo vkAppInfo = {};
	vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkAppInfo.pApplicationName = "Vulkan Testing";
	vkAppInfo.pEngineName = "Cryptic Engine";
	vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.apiVersion = VK_API_VERSION_1_0;
	vkAppInfo.pNext = nullptr;

	VkInstanceCreateInfo vkInstanceInfo = {};
	vkInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceInfo.pApplicationInfo = &vkAppInfo;

	// TODO: Read up on what exactly this means
	auto extensions = this->LoadRequiredExtensions();
	vkInstanceInfo.enabledExtensionCount = (uint32_t) extensions.size();
	vkInstanceInfo.ppEnabledExtensionNames = extensions.data();
	//

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	if (this->m_EnableValidationLayers)
	{

		LOG_INFO("Creating Debug Vulkan Instance...");
		vkInstanceInfo.enabledLayerCount = (uint32_t)this->m_ValidationLayers.size();
		vkInstanceInfo.ppEnabledLayerNames = this->m_ValidationLayers.data();

		SetupDebugMessengerInfo(createInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &createInfo;

	}
	else
	{

		vkInstanceInfo.enabledLayerCount = 0;
		vkInstanceInfo.ppEnabledLayerNames = nullptr;
		vkInstanceInfo.pNext = nullptr;
	}

	vkInstanceInfo.flags = 0;

	// Creating Vulkan Instance...
	if (vkCreateInstance(&vkInstanceInfo, nullptr, &this->m_VulkanInstance) != VK_SUCCESS)
	{
		LOG_CRITICAL("Failed to create a Vulkan instance!");
		return;
	}

	LOG_INFO("Successfully created a Vulkan instance!");

	uint32_t vkExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);
	std::vector<VkExtensionProperties> vkExtensions(vkExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, vkExtensions.data());

	LOG_TRACE("Available Vulkan Extensions:");
	for (const auto& e : vkExtensions)
		LOG_TRACE("\t{0}", e.extensionName);

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

			this->m_Device = device;
			break;
		}
	}

	if (!this->m_Device)
		LOG_CRITICAL("Failed to find a suitable device for vulkan!");
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