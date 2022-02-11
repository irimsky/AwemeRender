#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 screenPosition;


void main()
{
	screenPosition = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
//	if(gl_VertexID == 0) {
//		screenPosition = vec2(1.0, 2.0);
//		gl_Position = vec4(1.0, 3.0, 0.0, 1.0);
//	}
//	else if(gl_VertexID == 1) {
//		screenPosition = vec2(-1.0, 0.0);
//		gl_Position = vec4(-3.0, -1.0, 0.0, 1.0);
//	}
//	else if(gl_VertexID == 2) {
//		screenPosition = vec2(1.0, 0.0);
//		gl_Position = vec4(1.0, -1.0, 0.0, 1.0);
//	}

}
