struct VS_INPUT
{
	float3 pos : POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
	float2 tex : TEXCOORD0;
};

cbuffer ConstBuffer : register(b0)
{
	float4 colorMult;
};

cbuffer ConstBufferPerObj : register(b1)
{
	matrix world;
	matrix view;
	matrix proj;
}

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float4 pos = float4(input.pos, 1.0f);

	pos = mul(pos, world);
	pos = mul(pos, view);
	pos = mul(pos, proj);

	output.pos = pos;
	output.color = input.color * colorMult;
	output.tex = input.tex;
	return output;
}