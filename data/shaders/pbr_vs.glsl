#version 450 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

layout(binding=9) uniform sampler2D heightTexture;
uniform bool haveHeight;

layout(std140, binding=0) uniform TransformUniforms
{
	mat4 model;
	mat4 view;
	mat4 projection;
};

layout(location=0) out Vertex
{
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} vout;

void main()
{
	
	vout.texcoord = vec2(texcoord.x, 1.0-texcoord.y);
	vout.normal = mat3(model) *  normal;
	
	if(haveHeight)
	{
		vec4 dv = texture2D(heightTexture, vout.texcoord.xy);
		float df = dv.x;
		vec3 newPos = normal * df * 0.05 + position;
		vout.position = vec3(model * vec4(newPos, 1.0));
		gl_Position = projection * view * model * vec4(newPos, 1.0);
	}
	else{
		vout.position = vec3(model * vec4(position, 1.0));
		gl_Position = projection * view * model * vec4(position, 1.0);
	}
}
