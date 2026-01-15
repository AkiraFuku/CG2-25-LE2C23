#include "Sprite.hlsli"

struct Material
{
    float4 color; // 色 (Tint)
    float4x4 uvTransform; // UV変換行列
};

ConstantBuffer<Material> gMaterial : register(b0);

// スプライトはライティングしないため DirectionalLight は不要

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // UV変換 (UVスクロールや回転用)
    float4 transformedUV = mul(float4(input.texCoord, 0.0f, 1.0f), gMaterial.uvTransform);
    
    // テクスチャサンプリング
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    // 色の合成 (テクスチャ色 * マテリアル色)
    // ライティング計算(dot積など)は行わず、そのまま出力する
    output.color = gMaterial.color * textureColor;
    
    // アルファテスト (完全に透明なピクセルは描画しない場合)
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    return output;
}