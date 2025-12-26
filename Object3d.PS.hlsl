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
    float3 V = normalize(gCamera.worldPostion - input.worldPostion);
    
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
    
    float3 directionToPointLight = gPointLight.position - input.worldPostion;
    // 距離による減衰は計算せず、正規化して方向だけ使う
    float3 L_point = normalize(directionToPointLight);
    
    finalLighting += CalculateLight(N, L_point, V, gPointLight.color.rgb, gPointLight.intensity);

    // -----------------------------------------------------------
    // 結果の合成
    // -----------------------------------------------------------
    // テクスチャの色とライティング結果を乗算
    output.color.rgb = finalLighting * textureColor.rgb;
    output.color.a = gMaterial.Color.a * textureColor.a;

    return output;
    //PixelShaderOutput output;
    ////float3 toEye = normalize(gCamera.worldPostion - input.worldPostion);
    //float4 transformedUV = mul(float4(input.texCoord, 0.0f, 1.0f), gMaterial.uvTransform);
    //float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    //float3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
    //if (textureColor.a < 0.1f)
    //{
    //    discard; // 透明度が低いピクセルを破棄
    //}
    ////float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
    
    ////float NDotH = dot(normalize(input.normal), halfVector);
    //if (gMaterial.enableLighting == 0)
    //{
    //    output.color = gMaterial.Color * textureColor;
    //    return output;
    //}

    //// ベクトルの正規化
    //float3 N = normalize(input.normal);
    //float3 L = normalize(-gDirectionalLight.direction);
    //float3 V = normalize(gCamera.worldPostion - input.worldPostion);

    //// -----------------------------------------------------------
    //// 1. 拡散反射 (Diffuse) の計算
    //// -----------------------------------------------------------
    //float NdotL = dot(N, L);
    //float cos = saturate(NdotL);

    //// ハーフランバートへの切り替え用
    //if (gMaterial.diffuseType == 1)
    //{
    //    // Half-Lambert
    //    cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    //}
    //// Lambert (Default: diffuseType == 0) は上記の cos = saturate(NdotL) そのまま

    //float3 diffuse = gMaterial.Color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;


    //// -----------------------------------------------------------
    //// 2. 鏡面反射 (Specular) の計算
    //// -----------------------------------------------------------
    //float3 specular = float3(0.0f, 0.0f, 0.0f);
    //float specularPow = 0.0f;

    //if (gMaterial.specularType == 1)
    //{
    //    // Phong Reflection
    //    float3 R = reflect(-L, N);
    //    float RdotV = dot(R, V);
    //    specularPow = pow(saturate(RdotV), gMaterial.shininess);
    //    specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow;
    //}
    //else if (gMaterial.specularType == 2)
    //{
    //    // Blinn-Phong Reflection
    //    float3 H = normalize(L + V);
    //    float NdotH = dot(N, H);
    //    specularPow = pow(saturate(NdotH), gMaterial.shininess);
    //    specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow;
    //}
    //// specularType == 0 の場合は specular = 0 のまま


    //// -----------------------------------------------------------
    //// 3. 合成
    //// -----------------------------------------------------------
    //// 拡散反射 + 鏡面反射
    //output.color.rgb = diffuse + specular;
    //output.color.a = gMaterial.Color.a * textureColor.a;

    //return output;
}