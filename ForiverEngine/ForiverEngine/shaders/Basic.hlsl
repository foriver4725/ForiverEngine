cbuffer _0 : register(b0)
{
    float4x4 _Matrix_M_IT;
    float4x4 _Matrix_MVP;
}

Texture2DArray<float4> _Texture : register(t0);
SamplerState _Sampler : register(s0);

struct VSInput
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    uint texIndex : TEXINDEX;
};

struct V2P
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    nointerpolation uint texIndex : TEXINDEX;
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
    output.texIndex = input.texIndex;
    
    return output;
}

PSOutput PSMain(V2P input)
{
    PSOutput output;
    
    // 1枚のテクスチャに2つ分詰め込まれているので、それをアンパックする
    const uint odd = input.texIndex & 1;
    const float2 uvReal = odd ? input.uv + float2(0.0, 0.5) : input.uv;
    const uint texIndexReal = input.texIndex >> 1;
    
    output.color = _Texture.Sample(_Sampler, float3(uvReal, texIndexReal));
    
    return output;
}
