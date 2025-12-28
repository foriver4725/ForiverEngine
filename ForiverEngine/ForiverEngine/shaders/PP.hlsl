cbuffer _0 : register(b0)
{
    uint _WindowWidth;
    uint _WindowHeight;
    float _LimitLuminance;
    float _AAPower;
}

Texture2D<float4> _Texture : register(t0);
SamplerState _Sampler : register(s0);

struct VSInput
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct V2P
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

#include "./common/AA.hlsl"

V2P VSMain(VSInput input)
{
    V2P output;
    
    output.pos = input.pos;
    output.uv = input.uv;
    
    return output;
}

PSOutput PSMain(V2P input)
{
    PSOutput output;
    
    AAParams aaParams;
    aaParams.Texture = _Texture;
    aaParams.Sampler = _Sampler;
    aaParams.UV = input.uv;
    aaParams.UVPerPixel = float2(1.0 / _WindowWidth, 1.0 / _WindowHeight); // ポストプロセスなので、これでOK
    aaParams.LimitLuminance = _LimitLuminance;
    aaParams.AAPower = _AAPower;
    
    output.color = CalcAA(aaParams);
    
    return output;
}
