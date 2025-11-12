#include "Particle.hlsli"
struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};
StructuredBuffer<TransformationMatrix> gTransformationMatrix : register(t0);
//
struct VertexShaderInput
{

    float32_t4 position : POSITION0;
    float32_t2 texCoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input,uint32_t instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    
    output.position = mul(input.position, gTransformationMatrix[instanceId].WVP);
    output.texCoord = input.texCoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformationMatrix[instanceId].World));
    return output;
}