#include "Object3d.hlsli"

struct Material
{
    float32_t4 Color;
    int32_t enableLighting;
    float32_t4x4 uvTransform; // UV変換行列
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
    float4 transformedUV = mul(float32_t4(input.texCoord,0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    if (textureColor.a < 0.1f)
    {
        discard; // 透明度が低いピクセルを破棄
    }
    
    PixelShaderOutput output;
    
    
    if (gMaterial.enableLighting != 0)
    {
        float Ndotl = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(Ndotl * 0.5f + 0.5f, 2.0f);
        
        output.color.rgb = gMaterial.Color.rgb * textureColor.rgb*gDirectionalLight.color.rgb*cos*gDirectionalLight.intensity;
        output.color.a = gMaterial.Color.a * textureColor.a;
        
        if (output.color.a < 0.1f)
        {
            discard; // 透明度が低いピクセルを破棄
            
        }
        
    }
    else
    {
    output.color = gMaterial.Color*textureColor; // Red color
        if (output.color.a < 0.1f)
        {
            discard; // 透明度が低いピクセルを破棄
            
        }
    }
    
    return output;
}