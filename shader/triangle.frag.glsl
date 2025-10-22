//#version 450
//
//layout(location = 0) in vec4 inColor;
//layout(location = 0) out vec4 outColor;
//
//void main()
//{
//	outColor = inColor;
//}
//

#version 450

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 outColor;

void main()
{
	float u = inUV.x;
	float v = inUV.y;

	if (0.0 > u * u - v)
		outColor = inColor;
	else
		outColor = vec4(0.0);
}
