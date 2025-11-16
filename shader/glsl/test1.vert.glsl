#version 450

layout(set=0, binding=0) uniform Block {
	int value;
	int value2;
} ubo[10][9][8];

void main()
{
}
