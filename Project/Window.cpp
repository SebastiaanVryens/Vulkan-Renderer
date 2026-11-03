#include "Window.h"
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>

Window::Window( const int w, const int h, const std::string& name ) : m_Width{ w }, m_Height{ h }, m_WindowName{ name }
{
	InitWindow();
}

Window::~Window()
{
	glfwDestroyWindow( m_Window );
	glfwTerminate();
}

void Window::CreateWindowSurface( VkInstance instance, VkSurfaceKHR* surface )
{
	if ( glfwCreateWindowSurface( instance, m_Window, nullptr, surface ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to create window surface!" );
	}
}

void Window::UpdateFPS()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsedTime = currentTime - m_LastTime;
	m_FrameCount++;
	if ( elapsedTime.count() >= 1.0f ) 
	{
		m_FPS = m_FrameCount / elapsedTime.count();
		m_FPSString = "FPS: " + std::to_string( static_cast< int >( m_FPS ) );
		m_FrameCount = 0;
		m_LastTime = currentTime;
		std::string newTitle = m_WindowName + " - FPS: " + std::to_string( static_cast< int >( m_FPS ) );
		glfwSetWindowTitle( m_Window, newTitle.c_str() );
	}
}

void Window::InitWindow()
{
	glfwInit();
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE);

	m_Window = glfwCreateWindow( m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr );
	glfwSetWindowUserPointer( m_Window, this );
	glfwSetFramebufferSizeCallback( m_Window, FramebufferResizeCallback );
}

void Window::FramebufferResizeCallback( GLFWwindow* window, int width, int height )
{
	auto app = reinterpret_cast<Window*>( glfwGetWindowUserPointer( window ) );
	app->m_FrameBufferResized = true;
	app->m_Width = width;
	app->m_Height = height;
}
