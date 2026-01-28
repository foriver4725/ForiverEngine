cbuffer _0 : register(b0)
{
    int2 _PointerTextureSize;
    int2 _PointerPosition;
    float2 _PointerScale;
}

Texture2D<float4> _Texture : register(t0);
Texture2D<float4> _PointerTexture : register(t1);
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
    
    output.color = _Texture.Sample(_Sampler, input.uv);
    
    // 画面上の実寸値で、ポインタ画像が出てくるピクセルの範囲を求める
    const int2 pointerRealSize = (int2) floor((float2) _PointerTextureSize * _PointerScale);
    const int xMin = _PointerPosition.x - pointerRealSize.x / 2;
    const int xMax = xMin + pointerRealSize.x - 1;
    const int yMin = _PointerPosition.y - pointerRealSize.y / 2;
    const int yMax = yMin + pointerRealSize.y - 1;
    
    if (xMin <= input.pos.x && input.pos.x <= xMax &&
        yMin <= input.pos.y && input.pos.y <= yMax)
    {
        // ポインタ画像のUVを計算
        const float2 pointerUV = float2(
            (input.pos.x - xMin) / (float) pointerRealSize.x,
            (input.pos.y - yMin) / (float) pointerRealSize.y
        );
        
        const float4 pointerColor = _PointerTexture.Sample(_Sampler, pointerUV);
        
        // アルファ合成
        output.color = lerp(output.color, pointerColor, pointerColor.a);
    }
    
    return output;
}
