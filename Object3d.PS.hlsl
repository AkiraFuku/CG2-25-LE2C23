#include "Object3d.hlsli"

struct Material
{
    float4 Color;
    int enableLighting;
    int diffuseType; // 0:Lambert, 1:Half-Lambert
    int specularType; // 0:None, 1:Phong, 2:BlinnPhong

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
    float3 position; //ライトの向き
    float intensity; // 明るさ
    float radius;
    float decay;

};
struct SpotLight
{
    float4 color; //ライトの色
    float3 position; //ライトの向き
    float intensity; // 明るさ
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
   
};

struct Camera
{
    float3 worldPosition;
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


// -------------------------------------------------------------
// ライティング計算用関数
// N: 法線, L: ライトへの方向, V: カメラへの方向, lightColor: ライトの色, intensity: 強度
// -------------------------------------------------------------
float3 CalculateLight(float3 N, float3 L, float3 V, float3 lightColor, float intensity)
{
    // 1. 拡散反射 (Diffuse)
    float NdotL = dot(N, L);
    float cos = saturate(NdotL);
    if (gMaterial.diffuseType == 1) // Half-Lambert
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    float3 diffuse = gMaterial.Color.rgb * lightColor * cos * intensity;

    // 2. 鏡面反射 (Specular)
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float specularPow = 0.0f;

    if (gMaterial.specularType == 1) // Phong
    {
        float3 R = reflect(-L, N);
        float RdotV = dot(R, V);
        specularPow = pow(saturate(RdotV), gMaterial.shininess);
    }
    else if (gMaterial.specularType == 2) // Blinn-Phong
    {
        float3 H = normalize(L + V);
        float NdotH = dot(N, H);
        specularPow = pow(saturate(NdotH), gMaterial.shininess);
    }
    
    if (gMaterial.specularType != 0)
    {
        specular = lightColor * intensity * specularPow;
    }

    return diffuse + specular;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    
    
    PixelShaderOutput output;
    
    float4 textureColor = gTexture.Sample(gSampler, input.texCoord); // UV変換はVSで行っている前提、または必要ならここで計算

    // ライティングが無効ならそのまま返す
    if (gMaterial.enableLighting == 0)
    {
        output.color = gMaterial.Color * textureColor;
        return output;
    }

    float3 N = normalize(input.normal);
    float3 V = normalize(gCamera.worldPosition - input.worldPosition);
    
    float3 finalLighting = float3(0.0f, 0.0f, 0.0f);

    // -----------------------------------------------------------
    // Directional Light の計算
    // -----------------------------------------------------------
    float3 L_dir = normalize(-gDirectionalLight.direction); // 光源への方向
    finalLighting += CalculateLight(N, L_dir, V, gDirectionalLight.color.rgb, gDirectionalLight.intensity);

    // -----------------------------------------------------------
    // Point Light の計算 (減衰なし)
    // -----------------------------------------------------------
    // 頂点位置からライト位置へのベクトル
    
    float3 directionToPointLight = gPointLight.position - input.worldPosition;
    // 距離による減衰は計算せず、正規化して方向だけ使う
    float3 L_point = normalize(directionToPointLight);
    float distance = length(directionToPointLight);
    float factor = pow(saturate(-distance/gPointLight.radius+1.0f),gPointLight.decay);
    finalLighting += CalculateLight(N, L_point, V, gPointLight.color.rgb, gPointLight.intensity*factor);

    // -----------------------------------------------------------
    // 結果の合成
    // -----------------------------------------------------------
    // テクスチャの色とライティング結果を乗算
    output.color.rgb = finalLighting * textureColor.rgb;
    output.color.a = gMaterial.Color.a * textureColor.a;

    return output;
  
}