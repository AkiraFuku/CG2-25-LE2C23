#include "Object3d.hlsli"

struct Material
{
    float32_t4 Color;
};
ConstantBuffer<Material> gMaterial : register(b0);
struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
PixelShaderOutput main(VertexShaderOutput input)
{
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texCoord);
    PixelShaderOutput output;
    output.color = gMaterial.Color*textureColor; // Red color
    return output;
}