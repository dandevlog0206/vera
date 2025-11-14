struct FragmentIn
{
	float4 position : SV_Position;
	float4 color    : COLOR0;
};

[shader("fragment")]
float4 main(FragmentIn input) : SV_Target
{
	return input.color;
}