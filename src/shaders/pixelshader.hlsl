struct PointLight
{
	float4 pos;
	float3 att;
	float range;
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float specularPower;
};

struct Material
{
	float4 emissive;
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float specularPower;
	float specularIntensity;
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
	float4 eyePos;
};

cbuffer psMaterialBuffer : register(b1)
{
	Material mat;
};

Texture2D<float4> shaderTexture : register(t0);
SamplerState shaderSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float4 diffuse = mat.diffuse;

	float3 finalDiffuse = float3(0.0f, 0.0f, 0.0f);

	// Light pos <-> Pixel pos vector
	float3 lightToPixelVec = pointLight.pos.xyz - input.worldPos.xyz;

	// Light pos <-> Pixel pos distance
	float dist = length(lightToPixelVec);

	float3 finalAmbient = pointLight.ambient.xyz * mat.ambient.xyz;

	// Pixel too far?
	if (dist > pointLight.range)
		return float4(finalAmbient, diffuse.a);

	// Normalize light-pixel vec w.r.t. distance
	lightToPixelVec /= dist;

	// Calc the angle the light hits the pixel surface
	float lightIntensity = dot(lightToPixelVec, input.normal);

	float3 finalSpecular = float3(0.0f, 0.0f, 0.0f);

	float attenuation = pointLight.att[0] + (pointLight.att[1] * dist) + (pointLight.att[2] * (dist * dist));

	if (lightIntensity > 0.0f)
	{
		finalDiffuse += lightIntensity * diffuse.xyz * pointLight.diffuse.xyz;

		// Point light falloff factor
		finalDiffuse /= attenuation;
	}

	// Specular lighting using Phong lighting
	// From: http://ogldev.atspace.co.uk/www/tutorial19/tutorial19.html
	// + http://www.3dgep.com/texturing-lighting-directx-11/#Light_Properties
	float3 viewVec = eyePos.xyz - input.worldPos.xyz;
	float dist2 = length(viewVec);
	viewVec /= dist2;
	float3 lightReflect = normalize(reflect(-lightToPixelVec, input.normal));
	float specularFactor = dot(viewVec, lightReflect);

	if (specularFactor > 0.0f)
	{
		specularFactor = pow(specularFactor, pointLight.specularPower);
		finalSpecular = pointLight.diffuse.xyz * mat.specularIntensity * specularFactor;
		finalSpecular /= attenuation;
	}

	float3 finalEmissive = mat.emissive.xyz;

	float3 result = shaderTexture.Sample(shaderSampler, input.tex) * (finalEmissive + finalAmbient + finalDiffuse + finalSpecular);

	return float4(result, diffuse.a);
}