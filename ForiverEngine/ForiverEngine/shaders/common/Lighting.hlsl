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
    SamplerState Sampler;
    
    float3 WorldPosition;
    float4x4 SunMatrixVP; // 太陽の VP 行列
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
    // そもそも影を落とさない設定なら、ここで終了
    if (params.CastShadow < 0.5)
    {
        return 0.0;
    }
    
    // 太陽カメラから見たプロジェクション座標
    // ラスタライザを介さないので、ちゃんとw除算もする
    // 太陽カメラに映る範囲の場所について、この値の範囲は X[-1,1], Y[-1,1], Z[0,1]
    const float4 posAfterMVPFromSunRaw = mul(params.SunMatrixVP, float4(params.WorldPosition, 1.0));
    const float3 posAfterMVPFromSun = posAfterMVPFromSunRaw.xyz / posAfterMVPFromSunRaw.w;
    
    // テクスチャに書き込んであった深度値を取得
    // ただし、太陽カメラの範囲外にあるピクセルなら、ここで処理終了 (影を落とさない)
    // この値が、太陽から見て最も手前にあるピクセルの深度値である
    const float2 sunDepthTextureUV = posAfterMVPFromSun.xy * 0.5 + 0.5;
    if (sunDepthTextureUV.x < 0.0 || 1.0 < sunDepthTextureUV.x ||
        sunDepthTextureUV.y < 0.0 || 1.0 < sunDepthTextureUV.y)
    {
        return 0.0;
    }
    const float minDepthFromSun = params.SunDepthTexture.Sample(params.Sampler, sunDepthTextureUV);
    
    // このピクセル自体の、太陽から見た深度値を取得
    // 影を書き込んだ時の最前面ピクセルに影が落ちないように、ちょっとだけ深度を浅くして、判定を厳しくする
    const float depthFromSun = posAfterMVPFromSun.z * 0.999;
    
    // 深度を比較して、影を落とすかどうか判定する
    // 太陽から見た最前面ピクセルよりも奥にあるならば、影を落とす
    if (depthFromSun > minDepthFromSun)
    {
        return 1.0;
    }
    
    return 0.0;
}
