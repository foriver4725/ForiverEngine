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

// 戻り値の a は0/1 の2値で、テキストを描画するかどうかを示す
float4 PSGetTextColor(float2 pos)
{
    // 画面のピクセル座標 (画面いっぱいの板ポリなので、これで良いはず)
    const uint2 posAsInt = (uint2) pos;
    
    // フォントの種類を判別
    const uint2 windowTextIndex = posAsInt / _FontSingleLength;
    const uint4 windowTextData = _WindowTextTexture.Load(int3(windowTextIndex, 0));
    const uint windowTextIndexValue = windowTextData.a;
    const float3 windowTextColorValue = uint3(windowTextData.r, windowTextData.g, windowTextData.b) / 255.0;
    // 画面外なので、ここで終了
    if (any(windowTextIndex < 0 || _WindowTextTextureSize <= windowTextIndex))
        return float4(0, 0, 0, 0);
    // テキストを描画しないので、ここで終了
    if (windowTextIndexValue == _TextNothingIndex)
        return float4(0, 0, 0, 0);
    
    // フォントテクスチャにおける、フォントインデックスの最大値を算出 (ピクセル座標)
    const uint2 FontPixelIndexSize = _FontTextureSize / _FontSingleLength;
    // 描画するフォントの、テクスチャ内におけるUV座標を算出 (ピクセル座標. [0, _FontTextureSize-1])
    const uint2 fontPixelUV = uint2(windowTextIndexValue % FontPixelIndexSize.x, windowTextIndexValue / FontPixelIndexSize.x) * _FontSingleLength;
    // さらに、その中でいくらオフセットしているかを算出 (ピクセル座標. [0, _FontSingleLength-1])
    const uint2 fontPixelUVOffset = posAsInt % _FontSingleLength;
    // 実際に描画するべきテクセル値を取得
    const float4 font = _FontTexture.Load(int3(fontPixelUV + fontPixelUVOffset, 0));
    
    // 透明箇所だったら、描画しないのでここで終了
    if (font.a < 0.01)
        return float4(0, 0, 0, 0);
    
    // 色を返す
    return float4(windowTextColorValue, 1);
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
    
    const float4 originalColor = _Texture.Sample(_Sampler, input.uv);
    const float4 textColor = PSGetTextColor(input.pos.xy);
    
    // テキスト自体が不透明になることは無いので、強制的に上書きしてしまう
    output.color = textColor.a > 0.5 ? textColor : originalColor;
    
    return output;
}
