
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