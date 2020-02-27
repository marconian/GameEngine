
cbuffer WorldViewProjectionBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

cbuffer LightConstantBuffer : register(b1)
{
    float4 light;
};

struct VertexShaderInput
{
    float3 position : SV_POSITION;
    float3 normal   : NORMAL;
    float4 color    : COLOR;
    float2 tex      : TEXCOORD;
};

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR0;
    float3 normal   : NORMAL0;
    float3 vec      : POSITION0;
    float3 light    : POSITION1;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 position = float4(input.position, 1.0f);
    float4 normal = float4(input.normal, 1.0f);

    matrix mv = mul(world, view);

    // Transform the position from object space to homogeneous projection space
    position = mul(position, world);
    position = mul(position, view);
    position = mul(position, projection);

    float4 eye = float4(-20.0f, 0.0f, 0.0f, 1.0f);

    output.position = position;
    output.normal = mul(normal, mv).xyz;
    output.color = input.color;
    output.vec = -(eye - mul(input.tex, mv)).xyz;
    output.light = mul(light, mv).xyz;

	return output;
}