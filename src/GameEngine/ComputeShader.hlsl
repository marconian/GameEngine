//RWTexture2D<float> tex : register(u0);
RWStructuredBuffer<float> real : register(u0);
//Texture2D simpleTexture : register(t0);

cbuffer EnvironmentBuffer : register(b1)
{
    float3 light;
    float time;
};

[numthreads(64, 1, 1)]
void main(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
	real[groupId.x] = 1000.;
    //tex[dispatchThreadId.xy] = 1.0f;
}