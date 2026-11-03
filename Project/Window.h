#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vulkan/vulkan.h>
#include <chrono>

class Window
{
public:
	Window( const int w, const int h, const std::string& name );
	~Window();

	Window( const Window& ) = delete;
	Window( Window&& ) = delete;
	Window& operator=( const Window& ) = delete;
	Window& operator=( Window&& ) = delete;

	bool ShouldClose() { return glfwWindowShouldClose( m_Window ); }
	bool WasWindowResized() { return m_FrameBufferResized; }

	void ResetWindowResizedFlag() { m_FrameBufferResized = false; }

	void CreateWindowSurface( VkInstance instance, VkSurfaceKHR* surface );

	void UpdateFPS();

	VkExtent2D GetExtent() { return 
		{ static_cast<uint32_t>( m_Width ), 
		static_cast<uint32_t>( m_Height ) }; }

	GLFWwindow* GetGLFWwindow()const { return m_Window; }

private:
	void InitWindow();
	static void FramebufferResizeCallback( GLFWwindow* window, int width, int height );

	int m_Width;
	int m_Height;
	bool m_FrameBufferResized = false;

	std::string m_WindowName;
	GLFWwindow* m_Window;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_LastTime;
	int m_FrameCount = 0;
	float m_FPS = 0.0f;
	std::string m_FPSString;
};