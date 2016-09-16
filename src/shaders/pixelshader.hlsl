struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
	float2 tex : TEXCOORD0;
};

Texture2D<float4> shaderTexture : register(t0);
SamplerState shaderSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 texColor = shaderTexture.Sample(shaderSampler, input.tex) * input.color;
	return texColor;
}