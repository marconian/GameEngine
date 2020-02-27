Texture2D simpleTexture : register(t0);
SamplerState simpleSampler : register(s0);

cbuffer MaterialConstantBuffer : register(b2)
{
    float4 lightColor;
    float4 Ka;
    float4 Kd;
    float4 Ks;
    float4 shininess;
};

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR0;
    float3 normal   : NORMAL0;
    float3 vec      : POSITION0;
    float3 light    : POSITION1;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 L = normalize(input.light);
    float3 V = normalize(input.vec);
    float3 R = normalize(reflect(L, input.normal));
    
    float4 diffuse = Ka + (lightColor * Kd * max(dot(input.normal, L), 0.0f));
    diffuse = saturate(diffuse);
    
    float4 specular = Ks * pow(max(dot(R, V), 0.0f), shininess.x - 50.0f);
    specular = saturate(specular);
    
    float4 finalColor = input.color + diffuse + specular;
    
    return finalColor;

    //float3 lightDirection = normalize(float3(1, -1, 0));
    //float lightMagnitude = 0.8f * saturate(dot(input.normal, -lightDirection)) + 0.2f;
	//return input.color * lightMagnitude;
}