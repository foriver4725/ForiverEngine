#include <common/Defines.hlsl>

// フォントテクスチャからの、文字のサンプリング

struct TextSamplingParams
{
    Texture2D<float4> FontTexture; // フォントテクスチャ (ここからサンプリングする)
    Texture2D<uint4> WindowTextTexture; // 画面に表示する文字のデータ (各 [0, 255]. RGB: 色, A: フォントテクスチャでの、使う文字のインデックス)
    SamplerState Sampler;
    
    // テクスチャのサイズ
    uint2 FontTextureSize;
    uint2 WindowTextTextureSize;
    uint2 PixelPos; // 画面のピクセル座標 (画面いっぱいの板ポリなので、値の範囲は画面サイズと等しいはず)
    
    uint TextNothingIndex; // 画面に表示する文字のデータで、この値なら文字を描画しない (= デフォルト値)
    uint FontSingleLength; // フォントテクスチャにおける、1文字分のサイズ (px. 正方形)
};

// 戻り値の RGB : サンプリングした文字の色
// 戻り値の A : 文字を描画するかどうか (1.0 = 描画する, 0.0 = 描画しない)
float4 PSSampleText(TextSamplingParams params)
{
    // フォントテクスチャの中で、どの文字を描画するか、そのインデックスを取得する
    // また、結局文字を描画するピクセルでないならば、この段階で処理を終了できる
    
    const uint2 windowTextIndex = params.PixelPos / params.FontSingleLength;
    const uint4 windowTextData = params.WindowTextTexture.Load(int3(windowTextIndex, 0));
    //これは「フォントテクスチャにおける文字のインデックス」なので注意!
    const uint windowTextIndexValue = windowTextData.a;
    
    // 画面外なので、ここで終了
    if (any(windowTextIndex < 0 || params.WindowTextTextureSize <= windowTextIndex))
        return float4(0, 0, 0, 0);
    // テキストを描画しないので、ここで終了
    if (windowTextIndexValue == params.TextNothingIndex)
        return float4(0, 0, 0, 0);
    
    // フォントテクスチャからの、サンプリング処理コア部
    
    // フォントテクスチャにおける、フォントインデックスの最大値をxy毎に算出 (ピクセル座標)
    const uint2 fontPixelIndexSize = params.FontTextureSize / params.FontSingleLength; // y は使っていない
    // 描画するフォントの、フォントテクスチャ内におけるUV座標を算出 (ピクセル座標. 文字の左上座標. [0, params.FontTextureSize - 1])
    const uint2 fontPixelUV = uint2(windowTextIndexValue % fontPixelIndexSize.x, windowTextIndexValue / fontPixelIndexSize.x) * params.FontSingleLength;
    // さらに、その文字について、いくらオフセットした場所からサンプリングすれば良いかを算出 (ピクセル座標. [0, params.FontSingleLength - 1])
    const uint2 fontPixelUVOffset = params.PixelPos % params.FontSingleLength;
    // 以上を基に、実際に描画するべきテクセル値を取得
    const float4 font = params.FontTexture.Load(int3(fontPixelUV + fontPixelUVOffset, 0));
    
    // とはいうものの、結局フォントテクスチャの透明箇所だったら、描画しないのでここで終了
    if (font.a < 0.01)
        return float4(0, 0, 0, 0);
    
    // サンプリングした色を返す
    const float3 windowTextColorValue = windowTextData.rgb / 255.0;
    return float4(windowTextColorValue, 1);
}
