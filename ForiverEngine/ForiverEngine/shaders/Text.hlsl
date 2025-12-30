cbuffer _0 : register(b0)
{
    uint2 _FontTextureSize;
    uint2 _WindowTextTextureSize;
    uint _TextNothingIndex;
    uint _FontSingleLength;
}

Texture2D<float4> _Texture : register(t0);
Texture2D<float4> _FontTexture : register(t1);
Texture2D<uint4> _WindowTextTexture : register(t2);
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

#include <common/TextSampling.hlsl>

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
    
    const float4 originalColor = _Texture.Sample(_Sampler, input.uv);
    
    TextSamplingParams textSamplingParams;
    textSamplingParams.FontTexture = _FontTexture;
    textSamplingParams.WindowTextTexture = _WindowTextTexture;
    textSamplingParams.Sampler = _Sampler;
    textSamplingParams.FontTextureSize = _FontTextureSize;
    textSamplingParams.WindowTextTextureSize = _WindowTextTextureSize;
    textSamplingParams.PixelPos = uint2(input.pos.xy);
    textSamplingParams.TextNothingIndex = _TextNothingIndex;
    textSamplingParams.FontSingleLength = _FontSingleLength;
    
    const float4 textColor = PSSampleText(textSamplingParams);
    // テキスト自体が不透明になることは無いので、強制的に上書きしてしまう
    output.color = textColor.a > 0.5 ? textColor : originalColor;
    
    return output;
}
