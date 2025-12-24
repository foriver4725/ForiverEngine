cbuffer _0 : register(b0)
{
    float4x4 _Matrix_M;
    float4x4 _Matrix_M_IT;
    float4x4 _Matrix_MVP;
    
    float3 _SelectingBlockPosition;
    float _IsSelectingAnyBlock;
    float4 _SelectColor;
}

Texture2DArray<float4> _Texture : register(t0);
SamplerState _Sampler : register(s0);

struct VSInput
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 centerWorldPos : CENTERPOS;
    uint texIndex : TEXINDEX;
};

struct V2P
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
    nointerpolation float3 centerWorldPos : CENTERPOS;
    nointerpolation uint texIndex : TEXINDEX;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

float PSCheckIsSelectedBlock(float3 centerWorldPos)
{
    // そもそも選択中のブロックが無い
    if (_IsSelectingAnyBlock < 0.5)
    {
        return 0.0;
    }
    
    if (all(abs(centerWorldPos - _SelectingBlockPosition) < float3(0.01, 0.01, 0.01)))
    {
        return 1.0;
    }
    return 0.0;
}

V2P VSMain(VSInput input)
{
    V2P output;
    
    output.pos = mul(_Matrix_MVP, input.pos);
    output.normal = mul((float3x3) _Matrix_M_IT, input.normal);
    output.worldPos = mul(_Matrix_M, input.pos).xyz;
    
    output.uv = input.uv;
    output.centerWorldPos = input.centerWorldPos;
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
    
    float4 color = _Texture.Sample(_Sampler, float3(uvReal, texIndexReal));
    if (PSCheckIsSelectedBlock(input.centerWorldPos) > 0.5)
        color.rgb = lerp(color.rgb, _SelectColor.rgb, _SelectColor.a);
    
    output.color = color;
    
    return output;
}
