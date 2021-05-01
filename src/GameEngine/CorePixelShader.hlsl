#include "Globals.hlsli"

Texture2D simpleTexture : register(t0);
SamplerState simpleSampler : register(s0);

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 tex : TEXCOORD;
	float3 eye : POSITION0;
	float3 light : POSITION1;
	float3 local : POSITION2;
	float level : DEPTH;

	Instance instance;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 color = simpleTexture.SampleLevel(simpleSampler, input.tex, 0);
	return color;
}
