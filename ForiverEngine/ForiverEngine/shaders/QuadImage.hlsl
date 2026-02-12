// メッシュの座標は、スクリーン座標系 (NDC) で与えられることを想定

cbuffer _0 : register(b0)
{
    // 0 = disabled, 1 = enabled
    uint _IsDrawEnabled;
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
    
    output.color = (_IsDrawEnabled == 0) ?
        float4(0, 0, 0, 0) :
        _Texture.Sample(_Sampler, input.uv);
    
    return output;
}
