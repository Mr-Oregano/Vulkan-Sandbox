
#include "Application.h"
#include "Log.h"

void Application::Run()
{
	InitVulkan();
	Update();
	DeInit();
}

void Application::InitVulkan()
{

	LOG_INFO("Initializing Vulkan!");

}

void Application::Update()
{
}

void Application::DeInit()
{
}

int main(void)
{

	util::Log::Init();
	LOG_INFO("Vulkan Testing");

	Application app;
	app.Run();

	return 0;

}