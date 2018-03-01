struct VS_INPUT
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : POSITON1;
};

cbuffer ConstBufferPerObj : register(b0)
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
	output.worldPos = pos;

	pos = mul(pos, view);
	pos = mul(pos, proj);

	output.pos = pos;
	output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	output.tex = input.tex;
	output.normal = mul(input.normal, world);
	return output;
}