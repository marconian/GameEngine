
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
    float3 light;
};

Buffer<float3> b_velocity;

struct VertexShaderInput
{
    float3 position : SV_POSITION;
    float3 normal   : NORMAL;
    float4 color_    : COLOR;
    float2 tex      : TEXCOORD;

    float3 center   : INST_POSITION;
    float radius    : INST_RADIUS;
    float4 color    : INST_COLOR;
    float3 tidal    : INST_TIDAL;
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

float3 TidalForce(float3 position, float3 radius, float3 tidal)
{
    float3 _tidal = tidal * radius * 2;
    float3 _pull = normalize(position) * abs(_tidal);

    //float3 _pull = normalize(input.position) * abs(normalize(pull));
    //float _max_pull = max(abs(_pull), 0);
    //if (_max_pull > 0)
    //    _position += _pull * .1f;

    return _pull;
}

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;


    float3 _center = input.center;
    float3 _model = input.position * input.radius;
    float3 _position = _model + _center;
    float3 _eye = normalize(eye - _position);
    float3 _light = normalize(light - _position);

    //float3 _tidal = TidalForce(_position, input.radius, input.tidal);
    //_position += _tidal;

    output.position = mul(float4(_position, 1.), mvp);
    output.world = mul(float4(_position, 1.), m).xyz;
    output.normal = mul(float4(input.normal, 1.), m).xyz;
    output.eye = mul(float4(_eye, 1.), m).xyz;
    output.light = mul(float4(_light, 1.), m).xyz;
    output.color = input.color;
    output.tex = input.tex;

	return output;
}