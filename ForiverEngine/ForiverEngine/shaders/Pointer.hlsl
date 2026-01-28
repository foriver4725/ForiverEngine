cbuffer _0 : register(b0)
{
    float4 _PointerColor;
    uint2 _WindowSize;
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
    
    if (PSIsPointerPixel(uint2(input.pos.xy)) == 1)
    {
        output.color = _PointerColor;
        return output;
    }
    
    output.color = _Texture.Sample(_Sampler, input.uv);
    
    return output;
}
