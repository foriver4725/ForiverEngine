cbuffer _0 : register(b0)
{
    uint _WindowWidth;
    uint _WindowHeight;
    float _LimitLuminance;
    float _AAPower;
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

// ピクセルの輝度を計算する
float PSCalcLuminance(float4 color)
{
    return dot(color.rgb, float3(0.299, 0.587, 0.114));
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
    
    // ポストプロセスなので、この計算で OK
    const float2 uvPerPixel = float2(1.0 / _WindowWidth, 1.0 / _WindowHeight);
    
    // 近傍のピクセル値を取得する
    const float4 pixels[9] =
    {
        _Texture.Sample(_Sampler, input.uv + uvPerPixel * float2(-1, -1)), // 左上
        _Texture.Sample(_Sampler, input.uv + uvPerPixel * float2(0, -1)), // 上
        _Texture.Sample(_Sampler, input.uv + uvPerPixel * float2(1, -1)), // 右上
        
        _Texture.Sample(_Sampler, input.uv + uvPerPixel * float2(-1, 0)), // 左
        _Texture.Sample(_Sampler, input.uv + uvPerPixel * float2(0, 0)), // 中央
        _Texture.Sample(_Sampler, input.uv + uvPerPixel * float2(1, 0)), // 右
        
        _Texture.Sample(_Sampler, input.uv + uvPerPixel * float2(-1, 1)), // 左下
        _Texture.Sample(_Sampler, input.uv + uvPerPixel * float2(0, 1)), // 下
        _Texture.Sample(_Sampler, input.uv + uvPerPixel * float2(1, 1)), // 右下
    };
    
    // 近傍のピクセルの輝度を計算する
    // [0.0, 1.0]
    const float luminances[9] =
    {
        PSCalcLuminance(pixels[0]), // 左上
        PSCalcLuminance(pixels[1]), // 上
        PSCalcLuminance(pixels[2]), // 右上
        
        PSCalcLuminance(pixels[3]), // 左
        PSCalcLuminance(pixels[4]), // 中央
        PSCalcLuminance(pixels[5]), // 右
        
        PSCalcLuminance(pixels[6]), // 左下
        PSCalcLuminance(pixels[7]), // 下
        PSCalcLuminance(pixels[8]), // 右下
    };
    
    // AAの値を決定するため、上下左右斜め方向に沿って、3ピクセル単位でピクセル値の平均を求める
    const float4 pixelMeans[12] =
    {
        (pixels[0] + pixels[1] + pixels[2]) / 3.0, // 上
        (pixels[3] + pixels[4] + pixels[5]) / 3.0, // 中央 横
        (pixels[6] + pixels[7] + pixels[8]) / 3.0, // 下
        
        (pixels[0] + pixels[3] + pixels[6]) / 3.0, // 左
        (pixels[1] + pixels[4] + pixels[7]) / 3.0, // 中央 縦
        (pixels[2] + pixels[5] + pixels[8]) / 3.0, // 右
        
        (pixels[0] + pixels[3] + pixels[1]) / 3.0, // 斜め 左上
        (pixels[6] + pixels[4] + pixels[2]) / 3.0, // 斜め 中央 右上-左下
        (pixels[7] + pixels[5] + pixels[8]) / 3.0, // 斜め 右下
        
        (pixels[2] + pixels[1] + pixels[5]) / 3.0, // 斜め 右上
        (pixels[0] + pixels[4] + pixels[8]) / 3.0, // 斜め 中央 左上-右下
        (pixels[3] + pixels[7] + pixels[6]) / 3.0, // 斜め 左下
    };
    
    // AAの方向を決定するため、上下左右方向に沿って、3ピクセル単位でピクセルの輝度の平均を求める
    // [0.0, 1.0]
    const float luminanceMeans[12] =
    {
        (luminances[0] + luminances[1] + luminances[2]) / 3.0, // 上
        (luminances[3] + luminances[4] + luminances[5]) / 3.0, // 中央 横
        (luminances[6] + luminances[7] + luminances[8]) / 3.0, // 下
        
        (luminances[0] + luminances[3] + luminances[6]) / 3.0, // 左
        (luminances[1] + luminances[4] + luminances[7]) / 3.0, // 中央 縦
        (luminances[2] + luminances[5] + luminances[8]) / 3.0, // 右
        
        (luminances[0] + luminances[3] + luminances[1]) / 3.0, // 斜め 左上
        (luminances[6] + luminances[4] + luminances[2]) / 3.0, // 斜め 中央 右上-左下
        (luminances[7] + luminances[5] + luminances[8]) / 3.0, // 斜め 右下
        
        (luminances[2] + luminances[1] + luminances[5]) / 3.0, // 斜め 右上
        (luminances[0] + luminances[4] + luminances[8]) / 3.0, // 斜め 中央 左上-右下
        (luminances[3] + luminances[7] + luminances[6]) / 3.0, // 斜め 左下
    };
    
    // 輝度の差が一定以上、かつ最も大きい方向について、適当な係数を用いて AA をかける
    
    // 輝度が最大の方向を調べる
    const float luminanceMeanDiffs[8] =
    {
        abs(luminanceMeans[0] - luminanceMeans[1]), // 上-中央
        abs(luminanceMeans[2] - luminanceMeans[1]), // 下-中央
        abs(luminanceMeans[3] - luminanceMeans[4]), // 左-中央
        abs(luminanceMeans[5] - luminanceMeans[4]), // 右-中央
        
        abs(luminanceMeans[6] - luminanceMeans[7]), // 斜め 左上-中央
        abs(luminanceMeans[8] - luminanceMeans[7]), // 斜め 右下-中央
        abs(luminanceMeans[9] - luminanceMeans[10]), // 斜め 右上-中央
        abs(luminanceMeans[11] - luminanceMeans[10]), // 斜め 左下-中央
    };
    uint maxDiffIndex = -1;
    float maxDiff = 0.0;
    for (uint i = 0; i < 8; i++)
    {
        if (luminanceMeanDiffs[i] >= _LimitLuminance)
        {
            if (maxDiff < luminanceMeanDiffs[i])
            {
                maxDiff = luminanceMeanDiffs[i];
                maxDiffIndex = i;
            }
        }
    }
    
    if (maxDiffIndex == -1)
    {
        // 輝度差が一定以下なら AA をかけない
        output.color = pixels[4]; // 中央のピクセル値
        return output;
    }
    
    // 輝度の差を何乗かして重みを決定する
    // この値を、周辺ピクセルの方の係数値とする
    const float aaWeight = pow(maxDiff, _AAPower);
    
    // AA をかけた結果を計算する
    float4 aa;
    if (maxDiffIndex == 0)
        aa = lerp(pixelMeans[0], pixelMeans[1], aaWeight); // 上-中央
    else if (maxDiffIndex == 1)
        aa = lerp(pixelMeans[2], pixelMeans[1], aaWeight); // 下-中央
    else if (maxDiffIndex == 2)
        aa = lerp(pixelMeans[3], pixelMeans[4], aaWeight); // 左-中央
    else if (maxDiffIndex == 3)
        aa = lerp(pixelMeans[5], pixelMeans[4], aaWeight); // 右-中央
    else if (maxDiffIndex == 4)
        aa = lerp(pixelMeans[6], pixelMeans[7], aaWeight); // 斜め 左上-中央
    else if (maxDiffIndex == 5)
        aa = lerp(pixelMeans[8], pixelMeans[7], aaWeight); // 斜め 右下-中央
    else if (maxDiffIndex == 6)
        aa = lerp(pixelMeans[9], pixelMeans[10], aaWeight); // 斜め 右上-中央
    else // maxDiffIndex == 7
        aa = lerp(pixelMeans[11], pixelMeans[10], aaWeight); // 斜め 左下-中央
    
    // a値は、元の値をそのまま使う
    output.color = float4(aa.rgb, pixels[4].a);
    
    return output;
}
