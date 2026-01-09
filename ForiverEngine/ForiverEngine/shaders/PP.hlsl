cbuffer _0 : register(b0)
{
    uint2 _WindowSize;
    float _LimitLuminance;
    float _AAPower;
    
    float4 _PointerColor;
    uint _PointerLength; // 奇数前提
    uint _PointerWidth; // 奇数前提
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

#include <common/AA.hlsl>

// そのピクセルが、ポインタを描画するピクセルかどうかを判定する
// ポインタのピクセルなら 1、そうでなければ 0 を返す
uint PSIsPointerPixel(uint2 pixelPos)
{
    // ウィンドウ中央の座標
    static const uint2 WindowCenter = _WindowSize / 2;
    // ポインタの半分の長さ・幅 (中心から端までの距離)
    static const uint HalfPointerLength = _PointerLength / 2;
    static const uint HalfPointerWidth = _PointerWidth / 2;
    
    const uint diffX = abs(int(pixelPos.x) - int(WindowCenter.x));
    const uint diffY = abs(int(pixelPos.y) - int(WindowCenter.y));
    
    // 十字の横部分
    if (diffX <= HalfPointerLength && diffY <= HalfPointerWidth)
    {
        return 1;
    }
    // 十字の縦部分
    if (diffX <= HalfPointerWidth && diffY <= HalfPointerLength)
    {
        return 1;
    }
    
    return 0;
}

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
    
    // 画面中央のポインタを描画
    if (PSIsPointerPixel(uint2(input.pos.xy)) == 1)
    {
        output.color = _PointerColor;
        return output;
    }
    
    AAParams aaParams;
    aaParams.Texture = _Texture;
    aaParams.Sampler = _Sampler;
    aaParams.UV = input.uv;
    aaParams.UVPerPixel = 1.0 / _WindowSize; // ポストプロセスなので、これでOK
    aaParams.LimitLuminance = saturate(_LimitLuminance);
    aaParams.AAPower = _AAPower;
    
    output.color = PSCalcAA(aaParams);
    
    return output;
}
