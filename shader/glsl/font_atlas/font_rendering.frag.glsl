#version 450

layout(location = 0) in  vec2  inUV;
layout(location = 1) in  vec4  inColor;
layout(location = 2) in  float inAttr;
layout(location = 0) out vec4  outColor;

void main()
{
	float u = inUV.x;
	float v = inUV.y;

	if (inAttr == 0.0)
		outColor = inColor;
	else if (inAttr * (u * u - v) < 0.0)
		outColor = inColor;
	else
		outColor = vec4(0.0);
}
