
cbuffer ModelViewProjectionBuffer : register(b0)
{
    matrix mv;
    matrix mvp;
};

cbuffer EnvironmentBuffer : register(b1)
{
    float3 position;
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
    float4 position_vs = float4(input.position, 1.0f);
    float4 position_c = float4(position, 1.0f);
    float4 normal = float4(input.normal, 1.0f);

    // Transform the position from object space to homogeneous projection space
    //position_vs = position_vs + position;
    position_vs = position_vs + position_c;
    position_vs = mul(position_vs, mvp);

    float4 eye = float4(-20.0f, 0.0f, 0.0f, 1.0f);

    output.position = position_vs;
    output.normal = mul(normal, mv).xyz;
    output.color = input.color;
    output.vec = -(eye - mul(input.tex, mv)).xyz;
    output.light = mul(light, mv).xyz;

	return output;
}