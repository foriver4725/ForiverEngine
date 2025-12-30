cbuffer _0 : register(b0)
{
    float4 _UVLimit; // x: UMin, y: VMin, z: UMax, w: VMax
}

Texture2D<float4> _Texture : register(t0);
Texture2D<float4> _FontTexture : register(t1);
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
float4 PSGetTextColor(float2 uv)
{
    // テキストを描画できるUVの範囲内でない
    if (uv.x < _UVLimit.x || _UVLimit.z < uv.x ||
        uv.y < _UVLimit.y || _UVLimit.w < uv.y)
    {
        return float4(0, 0, 0, 0);
    }
    
    // 適当に取得する...
    // フォントテクスチャのクリアカラーは (0,0,0,0) なので、取得したテクセル値をそのまま返せば良い
    return _FontTexture.Sample(_Sampler, uv / 8);
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
    const float4 textColor = PSGetTextColor(input.uv);
    
    // テキスト自体が不透明になることは無いので、強制的に上書きしてしまう
    output.color = textColor.a > 0.5 ? textColor : originalColor;
    
    return output;
}
