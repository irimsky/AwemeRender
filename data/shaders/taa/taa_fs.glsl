#version 450 core

layout(binding=0) uniform sampler2D currentColor;
layout(binding=1) uniform sampler2D previousColor;
layout(binding=2) uniform sampler2D velocityTexture;


in  vec2 screenPosition;
out vec4 outColor;


void main()
{
	vec3 nowColor = texture(currentColor, screenPosition).rgb;
	vec2 velocity = texture(velocityTexture, screenPosition).rg;
	vec2 offsetUV = clamp(screenPosition - velocity, 0, 1);
	vec3 preColor = texture(previousColor, offsetUV).rgb;
	outColor = vec4(velocity, 0, 1.0);
	float c = 0.05;
	outColor = vec4(c * nowColor + (1-c) * preColor, 1.0);
}
