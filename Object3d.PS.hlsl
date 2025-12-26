#include "Object3d.hlsli"

struct Material
{
    float4 Color;
    int enableLighting;
    int lightingType;
    float4x4 uvTransform;
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
    float3 pointLightDirection = normalize(input.worldPostion - gPointLight.postion);
    if (textureColor.a < 0.1f)
    {
        discard; // 透明度が低いピクセルを破棄
    }
    float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
    
    float NDotH = dot(normalize(input.normal), halfVector);
    PixelShaderOutput output;
    
    if (gMaterial.enableLighting != 0)
    {
        
        
        
        float3 N = normalize(input.normal);
        float3 L = normalize(-gDirectionalLight.direction);
        float3 V = normalize(toEye);

    // --- 1. 拡散反射の強さ (cos) を決める ---
        float NdotL = dot(N, L);
        float cos = 0.0f;

    // ハーフランバートの場合 (ID: 3)
        if (gMaterial.lightingType == 3)
        {
        // -1.0 ~ 1.0 の範囲を 0.0 ~ 1.0 に変換して二乗する
        // これにより、光が当たっていない裏側まで明るさが回り込む
            cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        }
        else
        {
        // 通常のランバート、フォン、ブリンフォン
        // 0以下（裏側）は真っ黒にする
            cos = saturate(NdotL);
        }

    // --- 2. スペキュラ (鏡面反射) を決める ---
        float3 specularDirectionalLight = float3(0, 0, 0);
        float specularPow = 0.0f;

        if (gMaterial.lightingType == 1)
        { // Phong
            float3 R = reflect(-L, N);
            float RdotV = dot(R, V);
            specularPow = pow(saturate(RdotV), gMaterial.shininess);
            specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow;
        }
        else if (gMaterial.lightingType == 2)
        { // Blinn-Phong
            float3 H = normalize(L + V);
            float NdotH = dot(N, H);
            specularPow = pow(saturate(NdotH), gMaterial.shininess);
            specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow;
        }
    // Lambert(0) と Half-Lambert(3) はスペキュラなしのまま

    // --- 3. 最終合成 ---
    // 計算した cos を使って拡散反射を求める
        float3 diffuseDirectionalLight = gMaterial.Color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;

        output.color.rgb = diffuseDirectionalLight + specularDirectionalLight;
        output.color.a = gMaterial.Color.a * textureColor.a;

    }
    else
    {
    // ライティングなし
        output.color = gMaterial.Color * textureColor;
    }

    return output;
}