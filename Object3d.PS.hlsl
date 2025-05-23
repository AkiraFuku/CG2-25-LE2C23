

struct Material
{
    float32_t4 Color;
};
ConstantBuffer<Material> gMaterial : register(b0);
struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main()
{
    PixelShaderOutput output;
    output.color = gMaterial.Color; // Red color
    return output;
}