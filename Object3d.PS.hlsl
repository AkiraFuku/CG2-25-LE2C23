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
    if (textureColor.a < 0.1f)
    {
        discard; // 透明度が低いピクセルを破棄
    }
    float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
    
    float NDotH = dot(normalize(input.normal), halfVector);
    PixelShaderOutput output;
    
    if (gMaterial.enableLighting != 0)
    {
    // 基本的なベクトル計算
        float3 N = normalize(input.normal);
        float3 L = normalize(-gDirectionalLight.direction); // ライトへの向き
        float3 V = normalize(toEye); // 視点への向き (toEye)

    // ランバート (拡散反射)
    // NとLの内積。0未満にならないようにsaturateまたはmax(0, ...)
        float NdotL = dot(N, L);
        float cos = saturate(NdotL);

    // 拡散反射光 (Diffuse)
    // ランバート、フォン、ブリンフォンすべてで共通して使用
        float3 diffuse = gMaterial.Color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;

    // 鏡面反射光 (Specular)
        float3 specular = float3(0, 0, 0);
        float specularPow = 0.0f;

    // --- ライティングモデルの切り替え ---
        if (gMaterial.lightingType == 0)
        {
        // [0: Lambert] スペキュラなし
            specular = float3(0, 0, 0);
        }
        else if (gMaterial.lightingType == 1)
        {
        // [1: Phong] 反射ベクトル R を使う
            float3 R = reflect(-L, N); // ライト方向を反射
            float RdotV = dot(R, V);
            specularPow = pow(saturate(RdotV), gMaterial.shininess);
        
            specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow;
        }
        else if (gMaterial.lightingType == 2)
        {
        // [2: Blinn-Phong] ハーフベクトル H を使う
            float3 H = normalize(L + V);
            float NdotH = dot(N, H);
            specularPow = pow(saturate(NdotH), gMaterial.shininess);
        
            specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow;
        }

    // 最終合成
        output.color.rgb = diffuse + specular;
        output.color.a = gMaterial.Color.a * textureColor.a;

    }
    else
    {
    // ライティング無効時
        output.color = gMaterial.Color * textureColor;
    }

    return output;
}