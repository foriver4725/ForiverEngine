#include <common/Defines.hlsl>

// ポストプロセスでAAを適用する

struct AAParams
{
    Texture2D<float4> Texture;
    SamplerState Sampler;
    
    float2 UV;
    float2 UVPerPixel;
    
    float LimitLuminance; // ピクセルがモデルの端にあると判断する輝度差の閾値 ([0.0, 1.0]. 小さいほどAAが多くかかる)
    float AAPower; // アンチエイリアスの強さ (大きいほどAAが強くかかる)
};

// 輝度を計算する
float CalcLuminance(float4 color)
{
    static const float3 LuminanceWeights = float3(0.299, 0.587, 0.114);
    return dot(color.rgb, LuminanceWeights);
}

// 計算部
// FXAA っぽい何か
float4 PSCalcAA(AAParams params)
{
    // 近傍のピクセル値を取得する
    const float4 pixels[9] =
    {
        params.Texture.Sample(params.Sampler, params.UV + params.UVPerPixel * float2(-1, -1)), // 左上
        params.Texture.Sample(params.Sampler, params.UV + params.UVPerPixel * float2(0, -1)), // 上
        params.Texture.Sample(params.Sampler, params.UV + params.UVPerPixel * float2(1, -1)), // 右上
        
        params.Texture.Sample(params.Sampler, params.UV + params.UVPerPixel * float2(-1, 0)), // 左
        params.Texture.Sample(params.Sampler, params.UV + params.UVPerPixel * float2(0, 0)), // 中央
        params.Texture.Sample(params.Sampler, params.UV + params.UVPerPixel * float2(1, 0)), // 右
        
        params.Texture.Sample(params.Sampler, params.UV + params.UVPerPixel * float2(-1, 1)), // 左下
        params.Texture.Sample(params.Sampler, params.UV + params.UVPerPixel * float2(0, 1)), // 下
        params.Texture.Sample(params.Sampler, params.UV + params.UVPerPixel * float2(1, 1)), // 右下
    };
    
    // 近傍のピクセルの輝度を計算する
    // [0.0, 1.0]
    const float luminances[9] =
    {
        CalcLuminance(pixels[0]), // 左上
        CalcLuminance(pixels[1]), // 上
        CalcLuminance(pixels[2]), // 右上
        
        CalcLuminance(pixels[3]), // 左
        CalcLuminance(pixels[4]), // 中央
        CalcLuminance(pixels[5]), // 右
        
        CalcLuminance(pixels[6]), // 左下
        CalcLuminance(pixels[7]), // 下
        CalcLuminance(pixels[8]), // 右下
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
        if (luminanceMeanDiffs[i] >= params.LimitLuminance)
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
        return pixels[4]; // 中央のピクセル値
    }
    
    // 輝度の差を何乗かして重みを決定する
    // この値を、周辺ピクセルの方の係数値とする
    const float aaWeight = pow(maxDiff, params.AAPower);
    
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
    return float4(aa.rgb, pixels[4].a);
}
