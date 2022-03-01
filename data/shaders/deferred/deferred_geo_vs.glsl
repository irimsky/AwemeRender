#version 450 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

layout(binding=6) uniform sampler2D heightTexture;
uniform bool haveHeight;

layout(std140, binding=0) uniform TransformUniforms
{
	mat4 model;
	mat4 view;
	mat4 projection;
};

layout(std140, binding=1) uniform TaaUniforms
{
	mat4 preProjection;
	mat4 preView;
	mat4 preModel;

	int offsetIdx;
	float screenWidth;
	float screenHeight;
};

layout(location=0) out Vertex
{
	vec3 position;
	vec2 texcoord;
	vec3 normal;

	vec2 jitter;
	vec4 preScreenPosition;
	vec4 nowScreenPosition;
} vout;

const vec2 Halton_2_3[8] =
{
	vec2(0.0f, -1.0f / 3.0f),
	vec2(-1.0f / 2.0f, 1.0f / 3.0f),
	vec2(1.0f / 2.0f, -7.0f / 9.0f),
	vec2(-3.0f / 4.0f, -1.0f / 9.0f),
	vec2(1.0f / 4.0f, 5.0f / 9.0f),
	vec2(-1.0f / 4.0f, -5.0f / 9.0f),
	vec2(3.0f / 4.0f, 1.0f / 9.0f),
	vec2(-7.0f / 8.0f, 7.0f / 9.0f)
};

void main()
{
	vout.texcoord = vec2(texcoord.x, 1.0-texcoord.y);
	vout.normal = mat3(model) *  normal;
	
	float deltaWidth = 1.0/screenWidth, deltaHeight = 1.0/screenHeight;
	vec2 jitter = vec2(Halton_2_3[offsetIdx].x * deltaWidth, Halton_2_3[offsetIdx].y * deltaHeight);
	vout.jitter = jitter;
	
	mat4 jitterMat = mat4( 1.0 );
	jitterMat[3][0] += jitter.x;
	jitterMat[3][1] += jitter.y;
	
	vec3 nowPositon;
	if(haveHeight)
	{
		vec4 dv = texture2D(heightTexture, vout.texcoord.xy);
		float df = dv.x;
		nowPositon = normal * df * 0.05 + position;
	}
	else{
		nowPositon = position;
	}

	gl_Position = jitterMat * projection * view * model * vec4(nowPositon, 1.0);
	vout.position = vec3(model * vec4(nowPositon, 1.0));
	vout.preScreenPosition = preProjection * preView * preModel * vec4(nowPositon, 1.0);
	vout.nowScreenPosition = projection * view * model * vec4(nowPositon, 1.0);
}
