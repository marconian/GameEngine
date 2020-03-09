Texture2D simpleTexture : register(t0);
SamplerState simpleSampler : register(s0);

cbuffer MaterialConstantBuffer : register(b2)
{
    float4 lightColor;
    float3 Ka;
    float3 Kd;
    float3 Ks;
    float alpha;
};

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
    float3 normal   : NORMAL;
    float2 tex      : TEXCOORD;
    float3 world    : POSITION0;
    float3 eye      : POSITION1;
    float3 light    : POSITION2;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 V = normalize(input.eye);
    float3 L = normalize(input.light); // per light source
    float3 R = normalize(2 * max(dot(L, N), 0.f) * N - L); // per light source
    //float3 R = normalize(reflect(L, N));

    float3 id = float3(1, 1, 1); // per light source
    float3 is = float3(1, 1, 1); // per light source

    float3 Ip = Ka;

    float NdotL = dot(N, L);

    float3 diffuse = lightColor.xyz * Kd * max(dot(L, N), 0.f) * id;
    Ip += saturate(diffuse);

    float3 specular = Ks * pow(max(dot(R, V), 0.f), alpha) * is;
    Ip += saturate(specular);
    
    return input.color * float4(Ip, 1.f);
}