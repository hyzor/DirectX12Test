struct VS_INPUT
{
	float3 pos : POSITION;
	float4 color : COLOR;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
};

cbuffer ConstBuffer : register(b0)
{
	float4 colorMult;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = float4(input.pos, 1.0f);
	output.color = input.color * colorMult;
	return output;
}