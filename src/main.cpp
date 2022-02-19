#include <cstdio>
#include <string>
#include <memory>

#include "application.hpp"
#include "math.hpp"
#include "model.hpp"
#include "global.hpp"
#include <direct.h>

//#define MY_TEST

void init();

int main(int argc, char* argv[])
{
#ifdef MY_TEST
	Model t("E:\\Code\\OpenGL\\AwemeRender\\data\\models\\cerberus\\cerberus.obj");
	std::cout << t.name << std::endl;
	return 0;
#endif
	init();
	Renderer* renderer = new Renderer();
	try {
		Application().run(std::unique_ptr<Renderer>{ renderer });
	}
	catch (const std::exception& e) {
		std::fprintf(stderr, "Error: %s\n", e.what());
		return 1;
	}
}

void init()
{
	// 获取当前工作目录（后续当前工作目录可能被更改
	char* buffer;
	if ((buffer = _getcwd(NULL, 0)) == NULL)
		perror("getcwd error");
	else
		PROJECT_PATH = buffer;

	Application::sceneSetting.envNames = File::readAllFilesInDir(PROJECT_PATH + "\\data\\hdr");
	Application::sceneSetting.envName = new char[128];
	strcpy(Application::sceneSetting.envName, Application::sceneSetting.envNames[0]);
	Application::sceneSetting.preEnv = new char[128];
	strcpy(Application::sceneSetting.preEnv, Application::sceneSetting.envNames[0]);
	


	Application::sceneSetting.objectPitch = 0.0f;
	Application::sceneSetting.objectYaw = 0.0f;

	// 光照设置
	Application::sceneSetting.dirLights[0] = DirectionalLight(
		Math::vec3(1.0f), Math::vec3(-1.0f, 0.0f, 0.0f)
	);
	Application::sceneSetting.dirLights[1] = DirectionalLight(
		Math::vec3(1.0f), Math::vec3(1.0f, 0.0f, 0.0f)
	);
	Application::sceneSetting.dirLights[2] = DirectionalLight(
		Math::vec3(1.0f), Math::vec3(0.0f, -1.0f, 0.0f)
	);

	Application::sceneSetting.ptLights[0] = PointLight(
		Math::vec3(1.0f), Math::vec3(-15.0f, 0.0f, 0.0f)
	);
	Application::sceneSetting.ptLights[1] = PointLight(
		Math::vec3(1.0f), Math::vec3(15.0f, 0.0f, 0.0f)
	);
	Application::sceneSetting.ptLights[2] = PointLight(
		Math::vec3(1.0f), Math::vec3(0.0f,-15.0f, 0.0f)
	);

}
