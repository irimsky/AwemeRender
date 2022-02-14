#version 450 core

const float PI = 3.141592;
const float Epsilon = 0.00001;

const int NumLights = 3;

// 非金属的F0近似为0.4
const vec3 NonMetalF0 = vec3(0.04);

struct DirectionalLight {
	vec3 direction;
	vec3 radiance;
};

struct PointLight {
	vec3 position;
	vec3 radiance;
};

layout(location=0) in Vertex
{
	vec3 position;
	vec2 texcoord;
	vec3 normal;
	float height;
	vec3 oriPosition;
} vin;

layout(location=0) out vec4 color;


layout(std140, binding=1) uniform ShadingUniforms
{
	DirectionalLight dirLights[NumLights];
	PointLight ptLights[NumLights];
	vec3 eyePosition;
};

layout(binding=0) uniform sampler2D albedoTexture;
layout(binding=1) uniform sampler2D normalTexture;
layout(binding=2) uniform sampler2D metalnessTexture;
layout(binding=3) uniform sampler2D roughnessTexture;
layout(binding=4) uniform samplerCube specularTexture;
layout(binding=5) uniform samplerCube irradianceTexture;
layout(binding=6) uniform sampler2D specularBRDF_LUT;
layout(binding=7) uniform sampler2D occlusionTexture;
layout(binding=8) uniform sampler2D emmisiveTexture;
layout(binding=9) uniform sampler2D heightTexture;

uniform bool haveMetalness;
uniform bool haveRoughness;
uniform bool haveOcclusion;
uniform bool haveEmission;
uniform bool haveHeight;

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

// IBL中为了将kd从积分中取出来所做的近似
vec3 fresnelRoughness(vec3 F0, float cosTheta, float roughness)
{
	return F0 + (max(vec3(1-roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 getNewNormal()
{
	float height = vin.height;
	float du = dFdx(height);
	float dv = dFdy(height);
	vec3 newNormal = vec3(-du, -dv, 1);
	vec3 Q1  = dFdx(vin.oriPosition);
    vec3 Q2  = dFdy(vin.oriPosition);
    vec2 st1 = dFdx(vin.texcoord);
    vec2 st2 = dFdy(vin.texcoord);

    vec3 N  = normalize(vin.normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
	return normalize(TBN * newNormal);
}

vec3 getNormalFromMap()
{

    vec3 tangentNormal = texture(normalTexture, vin.texcoord).rgb * 2.0 - 1.0;

    vec3 Q1  = dFdx(vin.position);
    vec3 Q2  = dFdy(vin.position);
    vec2 st1 = dFdx(vin.texcoord);
    vec2 st2 = dFdy(vin.texcoord);
	
    vec3 N  = normalize(vin.normal);
	if(haveHeight)
		N = getNewNormal();
	
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
	
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}



void main()
{
	vec3 albedo = texture(albedoTexture, vin.texcoord).rgb;
	float metalness = 0.0;
	float roughness = 0.5;
	if(haveMetalness)
		metalness = texture(metalnessTexture, vin.texcoord).r;
	if(haveRoughness)
		roughness = texture(roughnessTexture, vin.texcoord).r;
	
	vec3 V = normalize(eyePosition - vin.position);
	vec3 N = getNormalFromMap();
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

		directLighting += (diffuseBRDF + specularBRDF) * Lradiance * NdotL;
	}

	// 点光源
	for(int i=0; i<NumLights; ++i)
	{
		vec3 L = ptLights[i].position - vin.position;
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
	vec3 irradiance = texture(irradianceTexture, N).rgb;

	vec3 Froughness = fresnelRoughness(F0, NdotV, roughness);
	vec3 kd = mix(vec3(1.0) - Froughness, vec3(0.0), metalness);
	// 1/PI 已经在预计算的过程中计算过了
	vec3 diffuseIBL = kd * albedo * irradiance;

	// 最大的Mipmap等级
	int specularTextureMaxLevels = textureQueryLevels(specularTexture);

	vec3 specularIrradiance = textureLod(specularTexture, R, roughness * specularTextureMaxLevels).rgb;
	vec2 specularBRDF = texture(specularBRDF_LUT, vec2(NdotV, roughness)).rg;

	vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

	ambientLighting = diffuseIBL + specularIBL;
	
	// 环境光遮蔽
	float AO = 1.0f;
	if(haveOcclusion)
		AO = texture(occlusionTexture, vin.texcoord).r;
	
	// 自发光项
	vec3 emmision = vec3(0);
	if(haveEmission)
		emmision = texture(emmisiveTexture, vin.texcoord).rgb;

	// 最终结果
	color = vec4(directLighting + AO * ambientLighting + emmision, 1.0);
}
