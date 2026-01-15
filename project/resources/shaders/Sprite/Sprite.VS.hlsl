#include "Sprite.hlsli"

struct TransformationMatrix
{
    float4x4 WVP; // World View Projection 行列
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texCoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    // 座標変換 (スプライトは通常、正射影行列などを使って変換されます)
    output.position = mul(input.position, gTransformationMatrix.WVP);
    
    // UV座標はそのまま渡す
    output.texCoord = input.texCoord;
    
    return output;
}