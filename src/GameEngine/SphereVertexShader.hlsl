
cbuffer ModelViewProjectionBuffer : register(b0)
{
    matrix m;
    matrix v;
    matrix p;
    matrix mv;
    matrix mp;
    matrix vp;
    matrix mvp;
    float3 eye;
};

cbuffer EnvironmentBuffer : register(b1)
{
    float3 position;
    float3 light;
    float3 pull;
};

struct VertexShaderInput
{
    float3 position : SV_POSITION;
    float3 normal   : NORMAL;
    float4 color    : COLOR;
    float2 tex      : TEXCOORD;

    float3 center   : INST_POSITION;
};

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
    float3 normal   : NORMAL;
    float3 world    : POSITION0;
    float3 eye      : POSITION1;
    float3 light    : POSITION2;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;

    //float3 _pull = max(input.position - normalize(pull), 0.);

    float3 _center = input.center;
    float3 _position = input.position + _center;
    float3 _eye = normalize(eye - _position);
    float3 _light = normalize(light - _position);

    output.position = mul(float4(_position, 1.), mvp);
    output.world = mul(float4(_position, 1.), m).xyz;
    output.normal = mul(float4(input.normal, 1.), m).xyz;
    output.eye = mul(float4(_eye, 1.), m).xyz;
    output.light = mul(float4(_light, 1.), m).xyz;
    output.color = input.color;

	return output;
}