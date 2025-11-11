#include "Particle.hlsli"

struct Material
{
    float32_t4 Color;
    int32_t enableLighting;
    float32_t4x4 uvTransform; // UV変換行列
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 transformedUV = mul(float32_t4(input.texCoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    output.color = gMaterial.Color * textureColor;
    if (textureColor.a < 0.1f)
    {
        discard; // 透明度が低いピクセルを破棄
    }
    
    
    
    
  
    
    return output;
}