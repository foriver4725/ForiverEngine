#include "./Defines.hlsl"

// 画像テクスチャからの、ピクセルカラーのサンプリング
// オフスクリーンレンダリング前提

struct ImageSamplingParams
{
    Texture2D<float4> ImageTexture; // 画像テクスチャ (ここからサンプリングする)
    SamplerState Sampler;
    
    uint2 TextureSize; // 画像テクスチャ全体のサイズ
    uint2 ClipMin; // 画像テクスチャの中で、どこからどこまでを切り取って使うか (min. テクスチャ自体のピクセル座標)
    uint2 ClipMax; // 画像テクスチャの中で、どこからどこまでを切り取って使うか (max. テクスチャ自体のピクセル座標)
    
    // 切り取ったテクスチャについて、その位置とスケール
    int2 Position; // 画像の中心座標
    float2 Scale;
    
    uint2 PixelPosition;
};

// 画像テクスチャのピクセルカラーをサンプリングして返す
// 画像テクスチャを表示するピクセル座標で無かったら、(0,0,0,0) を返す
float4 PSSampleImage(ImageSamplingParams params)
{
    // 画像をスクリーン上に配置するときの、そのピクセル座標の範囲を求める
    // 切り捨て
    const int2 imageRealSize = int2(
        floor((params.ClipMax.x - params.ClipMin.x + 1) * params.Scale.x),
        floor((params.ClipMax.y - params.ClipMin.y + 1) * params.Scale.y)
    );
    const int xMin = params.Position.x - imageRealSize.x / 2;
    const int xMax = xMin + imageRealSize.x - 1;
    const int yMin = params.Position.y - imageRealSize.y / 2;
    const int yMax = yMin + imageRealSize.y - 1;
    // 画像を表示するピクセルではなかったので、ここで終了
    if (params.PixelPosition.x < xMin || xMax < params.PixelPosition.x ||
        params.PixelPosition.y < yMin || yMax < params.PixelPosition.y)
    {
        return float4(0, 0, 0, 0);
    }
    
    // サンプリングしていく
    
    // 画像テクスチャのUV座標に変換
    // この段階では、切り取られた部分におけるUV座標になっている
    const float2 clippedImageUV = float2(
        (params.PixelPosition.x - xMin) / (float) imageRealSize.x,
        (params.PixelPosition.y - yMin) / (float) imageRealSize.y
    );
    // 元の画像テクスチャ全体におけるUV座標に変換する
    const float2 imageUV = float2(
        lerp(params.ClipMin.x / (float) params.TextureSize.x,
             (params.ClipMax.x + 1) / (float) params.TextureSize.x,
             clippedImageUV.x),
        lerp(params.ClipMin.y / (float) params.TextureSize.y,
             (params.ClipMax.y + 1) / (float) params.TextureSize.y,
             clippedImageUV.y)
    );
    
    const float4 color = params.ImageTexture.Sample(params.Sampler, imageUV);
    return color;
}
