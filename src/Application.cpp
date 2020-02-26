
#include "Application.h"
#include "Log.h"

void Application::Run()
{
	InitWindow();
	InitVulkan();
	Update();
	DeInit();
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

void Application::CreateVulkanInstance()
{

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
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions = NULL;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vkInstanceInfo.enabledExtensionCount = glfwExtensionCount;
	vkInstanceInfo.ppEnabledExtensionNames = glfwExtensions;
	//

	vkInstanceInfo.enabledLayerCount = 0;
	vkInstanceInfo.ppEnabledLayerNames = NULL;
	vkInstanceInfo.flags = 0;
	vkInstanceInfo.pNext = NULL;

	// Creating Vulkan Instance...
	if (vkCreateInstance(&vkInstanceInfo, nullptr, &this->vkInstance) != VK_SUCCESS)
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

void Application::InitVulkan()
{

	this->CreateVulkanInstance();

}

void Application::Update()
{

	glfwShowWindow(this->m_Window);
	while (!glfwWindowShouldClose(this->m_Window))
	{

		glfwPollEvents();

	}

}

void Application::DeInit()
{

	vkDestroyInstance(this->vkInstance, nullptr);

	glfwDestroyWindow(this->m_Window);
	glfwTerminate();

}

int main(void)
{

	util::Log::Init();
	LOG_INFO("Vulkan Testing");

	Application app;
	app.Run();

	return 0;

}