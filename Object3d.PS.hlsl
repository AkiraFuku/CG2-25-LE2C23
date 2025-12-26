#include "Object3d.hlsli"

struct Material
{
    float4 Color;
    int enableLighting;
    float4x4 uvTransform; // UV変換行列
    float shininess;
};
struct DirectionalLight
{
    float4 color; //ライトの色
    float3 direction; //ライトの向き
    float intensity; // 明るさ


};
struct PointLight
{
    float4 color; //ライトの色
    float3 postion; //ライトの向き
    float intensity; // 明るさ


};
struct Camera
{
    float3 worldPostion;
};

ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<PointLight> gPointLight : register(b3);
struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
PixelShaderOutput main(VertexShaderOutput input)
{
    float3 toEye = normalize(gCamera.worldPostion - input.worldPostion);
    float4 transformedUV = mul(float4(input.texCoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    float3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
    if (textureColor.a < 0.1f)
    {
        discard; // 透明度が低いピクセルを破棄
    }
    float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
    
    float NDotH = dot(normalize(input.normal), halfVector);
    PixelShaderOutput output;
    
    float specularPow;
    if (gMaterial.enableLighting != 0)
    {
        float Ndotl = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(Ndotl * 0.5f + 0.5f, 2.0f);
        
       // output.color.rgb = gMaterial.Color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
       // output.color.a = gMaterial.Color.a * textureColor.a;
        float RdotE = dot(reflectLight, toEye);
        //specularPow = pow(saturate(RdotE), gMaterial.shininess);
        specularPow = pow(saturate(NDotH), gMaterial.shininess);
        float3 diffuse =
    gMaterial.Color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        float3 speculer =
        gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        
        output.color.rgb = diffuse + speculer;
        output.color.a = gMaterial.Color.a * textureColor.a;
        if (output.color.a < 0.1f)
        {
            discard; // 透明度が低いピクセルを破棄
            
        }
        
    }
    else
    {
        output.color = gMaterial.Color * textureColor; // Red color
        if (output.color.a < 0.1f)
        {
            discard; // 透明度が低いピクセルを破棄
            
        }
    }
    
  
    
    return output;
}