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
	if (input.local.x == 0) return color;

	//if (input.color.w > 0) return input.color;

	Instance instance = input.instance;
	Material material = instance.material;

	float3 N = normalize(input.normal);
	float3 V = normalize(input.eye);
	float3 L = normalize(input.light); // per light source
	float3 R = normalize(2 * max(dot(L, N), 0.f) * N - L); // per light source

	float3 id = float3(1, 1, 1); // per light source
	float3 is = float3(1, 1, 1); // per light source

	//if (input.level < 0)
	//    id *= .1;

	float3 Ip = material.Ka;

	float NdotL = dot(N, L);

	float3 diffuse = material.Kd * max(dot(L, N), 0.f) * id;
	Ip += saturate(diffuse);

	float3 specular = material.Ks * pow(max(dot(R, V), 0.f), material.alpha) * is;
	Ip += saturate(specular);

	return color * float4(Ip, 1.f); // *float4(float3(0, 0, 0) + noise(input.tex.x, input.tex.y), 1.);
}
