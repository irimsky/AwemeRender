#pragma once
#include <GLFW/glfw3.h>
#include <stdexcept>

#include "application.hpp"
#include "global.hpp"


float Application::lastX = ScreenWidth / 2.0f;
float Application::lastY = ScreenHeight / 2.0f;
bool Application::firstMouse = true;
bool Application::showMouse = false;
float Application::deltaTime = 0.0f;
float Application::lastFrame = 0.0f;
float Application::lastFrameTime = 0.0f;
int Application::frameCount = 0;

SceneSettings Application::sceneSetting;
Camera Application::m_camera(glm::vec3(0.0f, 2.0f, 4.5f));

Application::Application()
	: m_window(nullptr)
{}

Application::~Application()
{
	if (m_window) {
		glfwDestroyWindow(m_window);
	}
	glfwTerminate();
}

void Application::run(const std::unique_ptr<Renderer>& renderer)
{
	m_window = renderer->initialize(ScreenWidth, ScreenHeight, DisplaySamples);

	glfwSetWindowPos(m_window, 50, 50);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetCursorPosCallback(m_window, Application::mousePositionCallback);
	glfwSetScrollCallback(m_window, Application::mouseScrollCallback);
	glfwSetKeyCallback(m_window, Application::keyCallback);
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(m_window);

	renderer->setup(sceneSetting);
	while (!glfwWindowShouldClose(m_window)) {

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		++frameCount;
		if (currentFrame - lastFrameTime >= 1.0f)
		{
			sceneSetting.FPS = frameCount / (currentFrame - lastFrameTime);
			frameCount = 0;
			lastFrameTime = currentFrame;
		}
		lastFrame = currentFrame;

		if(!showMouse) processInput();
		renderer->updateShadowMap(sceneSetting, m_camera);
		renderer->render(m_window, m_camera, sceneSetting);

		renderer->renderImgui(sceneSetting);
		
		
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	renderer->shutdown();
}

void Application::mousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (showMouse) return;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 

	lastX = xpos;
	lastY = ypos;

	m_camera.ProcessMouseMovement(xoffset, yoffset);
}

void Application::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (showMouse) return;
	m_camera.ProcessMouseScroll(yoffset);
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (action == GLFW_PRESS) {

		DirectionalLight* light = nullptr;

		// 隐藏/显示鼠标
		if (key == GLFW_KEY_LEFT_ALT) {
			showMouse = showMouse ? false : true;
			if (showMouse)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else
			{
				firstMouse = true;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
		}
		
		// 退出
		if (key == GLFW_KEY_ESCAPE) 
			glfwSetWindowShouldClose(window, true);

		// 灯光设置
		if (key == GLFW_KEY_F1)
			light = &sceneSetting.dirLights[0];
		if (key == GLFW_KEY_F2) 
			light = &sceneSetting.dirLights[1];
		if (key == GLFW_KEY_F3) 
			light = &sceneSetting.dirLights[2];
		
		
		if (light) {
			light->enabled = !light->enabled;
		}
	}
}

void Application::processInput()
{
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
		m_camera.ProcessKeyboard(FORWARD, deltaTime * MoveSpeed);
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
		m_camera.ProcessKeyboard(BACKWARD, deltaTime * MoveSpeed);
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
		m_camera.ProcessKeyboard(LEFT, deltaTime * MoveSpeed);
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
		m_camera.ProcessKeyboard(RIGHT, deltaTime * MoveSpeed);
	if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) {
		sceneSetting.objectPitch -= OrbitSpeed;
		if (sceneSetting.objectPitch < -180.0)
			sceneSetting.objectPitch += 360.0;
	}
	if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		sceneSetting.objectPitch += OrbitSpeed;
		if (sceneSetting.objectPitch > 180.0)
			sceneSetting.objectPitch -= 360.0;
	}
		
	if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		sceneSetting.objectYaw -= OrbitSpeed;
		if (sceneSetting.objectYaw < -180.0)
			sceneSetting.objectYaw += 360.0;
	}
		
	if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		sceneSetting.objectYaw += OrbitSpeed;
		if (sceneSetting.objectYaw > 180.0)
			sceneSetting.objectYaw -= 360.0;
	}
		
}
