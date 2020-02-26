#pragma once

#include <Windows.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <functional>

class Application
{
public:
	void Run();

private:
	void InitVulkan();
	void Update();
	void DeInit();

private:
	GLFWwindow *m_Window = NULL;

};