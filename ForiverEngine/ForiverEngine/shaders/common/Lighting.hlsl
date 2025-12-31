#include <common/Defines.hlsl>

// ライティング

struct LightingParams
{
    float3 Normal; // 座標変換後の法線ベクトル (正規化済み)
    
    float3 SunDirection; // 太陽光(平行光源) の方向 (正規化済み)
    float3 SunColor; // 太陽光(平行光源) の色
    
    float3 AmbientColor; // 環境光の色
};

struct ShadowParams
{
    float CastShadow; // 影を落とすか (1.0: 落とす, 0.0: 落とさない)
    Texture2D<float> SunDepthTexture; // 太陽からのカメラで書き込んだ、深度テクスチャ
    float4x4 SunVP; // 太陽の VP 行列
};

float3 PSCalcLighting(LightingParams params)
{
    // 太陽光
    const float NdotL = saturate(dot(params.Normal, -params.SunDirection));
    const float3 sun = params.SunColor * NdotL / PI;
    
    // 環境光
    const float3 ambient = params.AmbientColor;
    
    // 光の影響を合成して返す
    const float3 light = sun + ambient;
    return light;
}

// 戻り値: 1.0 (影を落とす), 0.0 (影を落とさない)
float PSCheckCastShadow(ShadowParams params)
{
    return 0;
}
