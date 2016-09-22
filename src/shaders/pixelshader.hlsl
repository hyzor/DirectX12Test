struct PointLight
{
	float4 pos;
	float3 att;
	float range;
	float4 ambient;
	float4 diffuse;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : POSITON1;
};

cbuffer psBuffer : register(b0)
{
	PointLight pointLight;
};

Texture2D<float4> shaderTexture : register(t0);
SamplerState shaderSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float4 diffuse = shaderTexture.Sample(shaderSampler, input.tex);

	float3 finalColor = float3(0.0f, 0.0f, 0.0f);

	// Light pos <-> Pixel pos vector
	float3 lightToPixelVec = pointLight.pos - input.worldPos;

	// Light pos <-> Pixel pos distance
	float dist = length(lightToPixelVec);

	float3 finalAmbient = diffuse * pointLight.ambient;

	// Pixel too far?
	if (dist > pointLight.range)
		return float4(finalAmbient, diffuse.a);

	// Normalize light-pixel vec w.r.t. distance
	lightToPixelVec /= dist;

	// Calc the angle the light hits the pixel surface
	float lightIntensity = dot(lightToPixelVec, input.normal);

	if (lightIntensity > 0.0f)
	{
		finalColor += lightIntensity * diffuse * pointLight.diffuse;

		// Point light falloff factor
		finalColor /= pointLight.att[0] + (pointLight.att[1] * dist) + (pointLight.att[2] * (dist * dist));
	}

	finalColor = saturate(finalColor + finalAmbient);

	//float4 texColor = shaderTexture.Sample(shaderSampler, input.tex) * input.color;
	//return texColor;

	return float4(finalColor, diffuse.a);
}