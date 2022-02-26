#version 450 core

layout(location=0) in Vertex
{
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} vin;

layout(location=0) out vec3 gPos;
layout(location=1) out vec3 gNormal;
layout(location=2) out vec3 gAlbedo;
layout(location=3) out vec3 gRMO;
layout(location=4) out vec3 gEmmis;

uniform vec3 commonColor;


layout(binding=0) uniform sampler2D albedoTexture;
layout(binding=1) uniform sampler2D normalTexture;
layout(binding=2) uniform sampler2D metalnessTexture;
layout(binding=3) uniform sampler2D roughnessTexture;
layout(binding=4) uniform sampler2D occlusionTexture;
layout(binding=5) uniform sampler2D emmisiveTexture;
layout(binding=6) uniform sampler2D heightTexture;

uniform bool haveAlbedo;
uniform bool haveNormal;
uniform bool haveMetalness;
uniform bool haveRoughness;
uniform bool haveOcclusion;
uniform bool haveEmission;
uniform bool haveHeight;

uniform bool haveSkybox;
uniform vec3 backgroundColor;

vec3 getNormalFromMap();

void main()
{
	// position
	gPos = vin.position;

	// normal
	vec3 N;
	if(haveNormal)
		N = getNormalFromMap();
	else
		N = normalize(vin.normal);
	gNormal = N;

	// basecolor
	vec3 albedo;
	if(haveAlbedo)
		albedo = texture(albedoTexture, vin.texcoord).rgb;
	else
		albedo = commonColor;
	gAlbedo = albedo;

	// RMO
	float metalness = 0.0;
	float roughness = 1.0;
	if(haveMetalness)
		metalness = texture(metalnessTexture, vin.texcoord).r;
	if(haveRoughness)
		roughness = texture(roughnessTexture, vin.texcoord).r;

	float AO = 1.0f;
	if(haveOcclusion)
		AO = texture(occlusionTexture, vin.texcoord).r;
	gRMO = vec3(roughness, metalness, AO);

	// emmision
	vec3 emmision = vec3(0.0);
	if(haveEmission)
		emmision = texture(emmisiveTexture, vin.texcoord).rgb;
	gEmmis = emmision;
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalTexture, vin.texcoord).rgb * 2.0 - 1.0;

    vec3 Q1  = dFdx(vin.position);
    vec3 Q2  = dFdy(vin.position);
    vec2 st1 = dFdx(vin.texcoord);
    vec2 st2 = dFdy(vin.texcoord);
	
    vec3 N  = normalize(vin.normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}
