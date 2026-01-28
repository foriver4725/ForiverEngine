#include "./Defines.hlsl"

// 画像テクスチャからの、ピクセルカラーのサンプリング
// オフスクリーンレンダリング前提

struct ImageSamplingParams
{
    Texture2D<float4> ImageTexture; // 画像テクスチャ (ここからサンプリングする)
    SamplerState Sampler;
    
    uint2 ImageTextureSize;
    int2 ImagePosition;
    float2 ImageScale;
    
    uint2 PixelPosition;
};

// 画像テクスチャのピクセルカラーをサンプリングして返す
// 画像テクスチャを表示するピクセル座標で無かったら、(0,0,0,0) を返す
float4 PSSampleImage(ImageSamplingParams params)
{
    // 画像をスクリーン上に配置するときの、そのピクセル座標の範囲を求める
    // 切り捨て
    const int2 imageRealSize = (int2) floor(params.ImageTextureSize * params.ImageScale);
    const int xMin = params.ImagePosition.x - imageRealSize.x / 2;
    const int xMax = xMin + imageRealSize.x - 1;
    const int yMin = params.ImagePosition.y - imageRealSize.y / 2;
    const int yMax = yMin + imageRealSize.y - 1;
    
    // 画像を表示するピクセルではなかったので、ここで終了
    if (params.PixelPosition.x < xMin || xMax < params.PixelPosition.x ||
        params.PixelPosition.y < yMin || yMax < params.PixelPosition.y)
    {
        return float4(0, 0, 0, 0);
    }
    
    // サンプリングしていく
    
    // 画像テクスチャのUV座標に変換
    const float2 imageUV = float2(
        (params.PixelPosition.x - xMin) / (float) imageRealSize.x,
        (params.PixelPosition.y - yMin) / (float) imageRealSize.y
    );
    
    const float4 color = params.ImageTexture.Sample(params.Sampler, imageUV);
    return color;
}
