struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION; // Position in clip space
    float32_t2 texCoord : TEXCOORD0; // Texture coordinates
    float32_t3 normal : NORMAL0;
    
};