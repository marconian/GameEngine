#include "Globals.hlsli"

RWTexture2D<float4> tex : register(u0);

cbuffer CursorBuffer : register(b0) { uint cursor; };
cbuffer SettingsBuffer : register(b1)
{
uint coreView;
};
cbuffer ColorProfileBuffer : register(b2)
{
float4 profile[180];
};

[numthreads(8, 8, 1)]
void main(
	uint3 groupId : SV_GroupID,
	uint3 groupThreadId : SV_GroupThreadID,
	uint3 dispatchThreadId : SV_DispatchThreadID,
	uint groupIndex : SV_GroupIndex)
{
	if (coreView > 0 && dispatchThreadId.x < 180)
	{
		float2 pos = float2(dispatchThreadId.x, (dispatchThreadId.y - 180.) * .5);
		uint dist = (uint)floor(distance(float2(0, 0), pos));
		if (dist > 179) dist = 179;

		tex[dispatchThreadId.xy] = profile[dist];
	}
	else
	{
		tex[dispatchThreadId.xy] = profile[179];
	}
}
