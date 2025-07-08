#include "Object3d.hlsli"

struct Material
{
    float32_t4 Color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};
struct DirectionalLight
{
    float32_t4 color; //ライトの色
    float32_t3 direction; //ライトの向き
    float intensity; // 明るさ


};
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
PixelShaderOutput main(VertexShaderOutput input)
{
    float3 TransformedUV = mul(float32_t3(input.texCoord,0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, TransformedUV.xy);
    PixelShaderOutput output;
    
    if (gMaterial.enableLighting != 0)
    {
        float Ndotl = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(Ndotl * 0.5f + 0.5f, 2.0f);
        output.color = gMaterial.Color * textureColor*gDirectionalLight.color*cos*gDirectionalLight.intensity;
    }
    else
    {
    output.color = gMaterial.Color*textureColor; // Red color
    }
    
    return output;
}