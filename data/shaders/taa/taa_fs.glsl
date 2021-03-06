#version 450 core

layout(binding=0) uniform sampler2D currentColor;
layout(binding=1) uniform sampler2D previousColor;
layout(binding=2) uniform sampler2D velocityTexture;
layout(binding=3) uniform sampler2D currentDepth;
layout(binding=4) uniform sampler2D previousDepth;

uniform float ScreenWidth;
uniform float ScreenHeight;
uniform int frameCount;

in  vec2 screenPosition;
out vec4 outColor;

vec3 RGB2YCoCgR(vec3 rgbColor)
{
	vec3 YCoCgRColor;

	YCoCgRColor.y = rgbColor.r - rgbColor.b;
	float temp = rgbColor.b + YCoCgRColor.y / 2;
	YCoCgRColor.z = rgbColor.g - temp;
	YCoCgRColor.x = temp + YCoCgRColor.z / 2;

	return YCoCgRColor;
}

vec3 YCoCgR2RGB(vec3 YCoCgRColor)
{
	vec3 rgbColor;

	float temp = YCoCgRColor.x - YCoCgRColor.z / 2;
	rgbColor.g = YCoCgRColor.z + temp;
	rgbColor.b = temp - YCoCgRColor.y / 2;
	rgbColor.r = rgbColor.b + YCoCgRColor.y;

	return rgbColor;
}

float Luminance(vec3 color)
{
	return 0.25 * color.r + 0.5 * color.g + 0.25 * color.b;
}

vec3 ToneMap(vec3 color)
{
	return color / (1 + Luminance(color));
}

vec3 UnToneMap(vec3 color)
{
	return color / (1 - Luminance(color));
}

vec2 getClosestOffset()
{
	vec2 deltaRes = vec2(1.0 / ScreenWidth, 1.0 / ScreenHeight);
	float closestDepth = 1.0f;
	vec2 closestUV = screenPosition;

	for(int i=-1;i<=1;++i)
	{
		for(int j=-1;j<=1;++j)
		{
			vec2 newUV = screenPosition + deltaRes * vec2(i, j);

			float depth = texture2D(currentDepth, newUV).x;

			if(depth < closestDepth)
			{
				closestDepth = depth;
				closestUV = newUV;
			}
		}
	}

	return closestUV;
}

vec3 clipAABB(vec3 nowColor, vec3 preColor)
{
	vec3 aabbMin = nowColor, aabbMax = nowColor;
	vec2 deltaRes = vec2(1.0 / ScreenWidth, 1.0 / ScreenHeight);
	vec3 m1 = vec3(0), m2 = vec3(0);

	for(int i=-1;i<=1;++i)
	{
		for(int j=-1;j<=1;++j)
		{
			vec2 newUV = screenPosition + deltaRes * vec2(i, j);
			vec3 C = RGB2YCoCgR(ToneMap(texture(currentColor, newUV).rgb));
			aabbMax = max(aabbMax, C);
			aabbMin = min(aabbMin, C);
			m1 += C;
			m2 += C * C;
		}
	}

	// Variance clip
	const int N = 9;
	const float VarianceClipGamma = 1.0f;
	vec3 mu = m1 / N;
	vec3 sigma = sqrt(abs(m2 / N - mu * mu));
	aabbMin = mu - VarianceClipGamma * sigma;
	aabbMax = mu + VarianceClipGamma * sigma;

	// clip to center
	vec3 p_clip = 0.5 * (aabbMax + aabbMin);
	vec3 e_clip = 0.5 * (aabbMax - aabbMin);

	vec3 v_clip = preColor - p_clip;
	vec3 v_unit = v_clip.xyz / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if (ma_unit > 1.0)
		return p_clip + v_clip / ma_unit;
	else
		return preColor;// point inside aabb
	
	//clamp
//	preColor = clamp(preColor, aabbMin, aabbMax);
//	return preColor;
}

void main()
{
	vec3 nowColor = texture(currentColor, screenPosition).rgb;
	if(frameCount == 0)
	{
		outColor = vec4(nowColor, 1.0);
		return;
	}

	// ??????3x3??????????????????????????????
	vec2 velocity = texture(velocityTexture, getClosestOffset()).rg;
	vec2 offsetUV = clamp(screenPosition - velocity, 0, 1);
	vec3 preColor = texture(previousColor, offsetUV).rgb;
	
	nowColor = RGB2YCoCgR(ToneMap(nowColor));
	preColor = RGB2YCoCgR(ToneMap(preColor));
	
	preColor = clipAABB(nowColor, preColor);

	preColor = UnToneMap(YCoCgR2RGB(preColor));
	nowColor = UnToneMap(YCoCgR2RGB(nowColor));


	float c = 0.05 + length(velocity);
	outColor = vec4(c * nowColor + (1-c) * preColor, 1.0);
}
