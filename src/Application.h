#pragma once

#include <Windows.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <functional>
#include <vector>

class Application
{
public:
	void Run();

private:
	void InitWindow();

	void CreateVulkanInstance();
	void InitVulkan();

	void Update();
	void DeInit();

private:
	GLFWwindow *m_Window = NULL;
	VkInstance vkInstance;

};