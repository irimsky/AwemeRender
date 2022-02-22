#version 450 core
layout(location = 0) in vec3 aPos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

layout(binding=0) uniform sampler2D heightTexture;
uniform bool haveHeight;

void main()
{
	vec2 texcoord = vec2(texcoord.x, 1.0-texcoord.y);
	if(haveHeight)
	{
		vec4 dv = texture2D(heightTexture, texcoord.xy);
		float df = dv.x;
		vec3 newPos = normal * df * 0.05 + aPos;

		gl_Position = lightSpaceMatrix * model * vec4(newPos, 1.0);
	}
	else{
		gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
	}
}