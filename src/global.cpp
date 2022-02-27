#include "global.hpp"

std::string PROJECT_PATH;

const int ScreenWidth = 1700;
const int ScreenHeight = ScreenWidth * 9 / 16;
const int DisplaySamples = 16;
const float MoveSpeed = 2.5;
const float OrbitSpeed = 0.8;

const float Near = 0.1f;
const float Far = 50.0f;

// 各种贴图大小
const int EnvMapSize = 1024;	// 必须是2的次幂
const int IrradianceMapSize = 32;
const int BRDF_LUT_Size = 512;
const int ShadowMapSize = 2048;
