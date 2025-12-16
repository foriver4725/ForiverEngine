cbuffer _0 : register(b0)
{
    float4x4 _Matrix_M_IT;
    float4x4 _Matrix_MVP;
    int _TextureIndex;
    int _UseUpperUV;
}

Texture2DArray<float4> _Texture : register(t0);
SamplerState _Sampler : register(s0);

struct VSInput
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct V2P
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

V2P VSMain(VSInput input)
{
    V2P output;
    
    output.pos = mul(_Matrix_MVP, input.pos);
    output.normal = mul((float3x3) _Matrix_M_IT, input.normal);
    output.uv = input.uv;
    
    return output;
}

PSOutput PSMain(V2P input)
{
    PSOutput output;
    
    // 1枚のテクスチャのうち、上下どちらから読み取るべきかを判断する
    float2 uv = _UseUpperUV > 0.5 ? input.uv : input.uv + float2(0.0, 0.5);
    output.color = _Texture.Sample(_Sampler, float3(uv, _TextureIndex));
    
    return output;
}
