#version 450 core

const float PI = 3.141592;
const float Epsilon = 0.00001;
const int NumLights = 3;

// 非金属的F0近似为0.4
const vec3 NonMetalF0 = vec3(0.04);

struct DirectionalLight {
	vec3 direction;
	vec3 radiance;
	mat4 lightSpaceMatrix;
};

struct PointLight {
	vec3 position;
	vec3 radiance;
};

layout(std140, binding=1) uniform ShadingUniforms
{
	DirectionalLight dirLights[NumLights];
	PointLight ptLights[NumLights];
	vec3 eyePosition;
};

layout(location=0) out vec4 color;
in vec2 TexCoords;

layout(binding=0) uniform sampler2D gPos;
layout(binding=1) uniform sampler2D gNormal;
layout(binding=2) uniform sampler2D gAlbedo;
layout(binding=3) uniform sampler2D gRMO;
layout(binding=4) uniform sampler2D gEmmis;


layout(binding=5) uniform samplerCube specularTexture;
layout(binding=6) uniform samplerCube irradianceTexture;
layout(binding=7) uniform sampler2D specularBRDF_LUT;

layout(binding=8) uniform sampler2D dirLightShadowMap0;
layout(binding=9) uniform sampler2D dirLightShadowMap1;
layout(binding=10) uniform sampler2D dirLightShadowMap2;

layout(binding=11) uniform sampler2D depthMap;

uniform bool haveSkybox;
uniform vec3 backgroundColor;

float NDF_GGX(float cosLh, float roughness);
float gaSchlickG1(float cosTheta, float k);
float gaSchlickGGX(float NdotL, float NdotV, float roughness);

vec3 fresnelSchlick(vec3 F0, float cosTheta);
vec3 fresnelRoughness(vec3 F0, float cosTheta, float roughness);

float dirLightShadow(vec4 fragPosLightSpace, float cosTheta, sampler2D depthMap);
float dirLightVisibility(vec4 fragPosLightSpace, float cosTheta, int idx);


void main()
{
	// GBuffer中获取数据
	vec3 albedo = texture(gAlbedo, TexCoords).rgb;
	float roughness = texture(gRMO, TexCoords).r;
	float metalness = texture(gRMO, TexCoords).g;
	float AO = texture(gRMO, TexCoords).b;
	vec3 fragPos = texture(gPos, TexCoords).rgb;
	vec3 emmision = texture(gEmmis, TexCoords).rgb;
	vec3 N = texture(gNormal, TexCoords).rgb;

	vec3 V = normalize(eyePosition - fragPos);

	vec3 R = reflect(-V, N);

	float NdotV = max(0.0, dot(N, V));

	// 对于金属物体，其F0用金属度插值模拟
	vec3 F0 = mix(NonMetalF0, albedo, metalness);

	// 直接光照
	vec3 directLighting = vec3(0);
	// 平行光
	for(int i=0; i<NumLights; ++i)
	{
		vec3 L = -dirLights[i].direction;
		vec3 Lradiance = dirLights[i].radiance;
		vec3 H = normalize(V + L);

		float NdotL = max(0.0, dot(N, L));
		float NdotH = max(0.0, dot(N, H));
		float HdotV = max(0.0, dot(H, V));

		vec3  F = fresnelSchlick(F0, HdotV);
		float D = NDF_GGX(NdotH, roughness);
		float G = gaSchlickGGX(NdotL, NdotV, roughness);

		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// 漫反射部分
		vec3 diffuseBRDF = kd * albedo;

		// 高光部分
		// 防止除以0
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * NdotL * NdotV);
		vec4 fragPosLightSpace = dirLights[i].lightSpaceMatrix * vec4(fragPos, 1.0);
		float visibility = dirLightVisibility(fragPosLightSpace, NdotL, i);
		directLighting += ((diffuseBRDF + specularBRDF) * Lradiance * NdotL) * visibility;
	}

	// 点光源
	for(int i=0; i<NumLights; ++i)
	{
		vec3 L = ptLights[i].position - fragPos;
		float dis = length(L);
		L = normalize(L);
		// 200距离 1.0	0.022	0.0019
		float attenuation = 1.0 + 0.022 * dis + 0.0019 * dis * dis;
		vec3 Lradiance = ptLights[i].radiance;
		vec3 H = normalize(V + L);

		float NdotL = max(0.0, dot(N, L));
		float NdotH = max(0.0, dot(N, H));
		float HdotV = max(0.0, dot(H, V));

		vec3  F = fresnelSchlick(F0, HdotV);
		float D = NDF_GGX(NdotH, roughness);
		float G = gaSchlickGGX(NdotL, NdotV, roughness);

		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// 漫反射部分
		vec3 diffuseBRDF = kd * albedo;

		// 高光部分
		// 防止除以0
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * NdotL * NdotV);

		directLighting += (diffuseBRDF + specularBRDF) * Lradiance / attenuation * NdotL;
	}

	// 环境光照
	vec3 ambientLighting;
	vec3 irradiance = backgroundColor;
	if(haveSkybox)
		irradiance = texture(irradianceTexture, N).rgb;	
	vec3 Froughness = fresnelRoughness(F0, NdotV, roughness);
	vec3 kd = mix(vec3(1.0) - Froughness, vec3(0.0), metalness);
	// 1/PI 已经在预计算的过程中计算过了
	vec3 diffuseIBL = kd * albedo * irradiance;

	// 最大的Mipmap等级
	int specularTextureMaxLevels = textureQueryLevels(specularTexture);
	
	vec3 specularIrradiance = backgroundColor;
	if(haveSkybox)
		specularIrradiance = textureLod(specularTexture, R, roughness * specularTextureMaxLevels).rgb;
	vec2 specularBRDF = texture(specularBRDF_LUT, vec2(NdotV, roughness)).rg;

	vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

	ambientLighting = diffuseIBL + specularIBL;

	// 最终结果
//	gl_FragDepth = 0;
	color = vec4(directLighting + AO * ambientLighting + emmision, 1.0);
//	color = vec4(vec3(texture(depthMap, TexCoords).r), 1.0f);
}

float NDF_GGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}


float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

float gaSchlickGGX(float NdotL, float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic在论文中建议用在直接光部分
	return gaSchlickG1(NdotL, k) * gaSchlickG1(NdotV, k);
}

vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelRoughness(vec3 F0, float cosTheta, float roughness)
{
//	color = vec4(F0 + (max(vec3(1-roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0), 1.0);
	return F0 + (max(vec3(1-roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec2 poissonDisk[16] = {
    vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ),
    vec2( -0.094184101, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ),
    vec2( -0.91588581, 0.45771432 ),
    vec2( -0.81544232, -0.87912464 ),
    vec2( -0.38277543, 0.27676845 ),
    vec2( 0.97484398, 0.75648379 ),
    vec2( 0.44323325, -0.97511554 ),
    vec2( 0.53742981, -0.47373420 ),
    vec2( -0.26496911, -0.41893023 ),
    vec2( 0.79197514, 0.19090188 ),
    vec2( -0.24188840, 0.99706507 ),
    vec2( -0.81409955, 0.91437590 ),
    vec2( 0.19984126, 0.78641367 ),
    vec2( 0.14383161, -0.14100790 )
};

float dirLightShadow(vec4 fragPosLightSpace, float cosTheta, sampler2D depthMap)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    if(projCoords.z > 1.0)
        return 1.0;
    projCoords = projCoords * 0.5 + 0.5;
	float closestDepth;
    
    float currentDepth = projCoords.z;
    
	float bias = max(0.05 * (1.0 - cosTheta), 0.005);
	float visibility = 0;

	for(int i=0;i<16;++i)
	{
		float angle = 2.0 * PI * fract(sin(fragPosLightSpace.x)*10000.0);
		float s = sin(angle);
        float c = cos(angle);
		vec2 randOffset = vec2(
			poissonDisk[i].x * c + poissonDisk[i].y * s,
			poissonDisk[i].x * (-s) + poissonDisk[i].y * c
		);
		float pcfDepth = texture(depthMap, projCoords.xy + randOffset * texelSize).r; 
		visibility += smoothstep(currentDepth - bias, currentDepth - 0.5 * bias, pcfDepth);
	}
	visibility /= 16.0;
	return visibility;
}

float dirLightVisibility(vec4 fragPosLightSpace, float cosTheta, int idx)
{
	if(idx == 0) 
		return dirLightShadow(fragPosLightSpace, cosTheta, dirLightShadowMap0);
    else if(idx == 1)
		return dirLightShadow(fragPosLightSpace, cosTheta, dirLightShadowMap1);
	else if(idx == 2)
		return dirLightShadow(fragPosLightSpace, cosTheta, dirLightShadowMap2);
}
