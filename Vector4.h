#pragma once
struct Vector4{
    float x;
    float y;
    float z;
    float w;
};
struct Matrix4x4{
   float m[4][4];
};
struct Vector2{
    float x;
    float y;
};
struct Vector3{
    float x;
    float y;
    float z;
};
struct Transform{
    Vector3 scale;
    Vector3 rotate;
    Vector3 traslate;
};
struct VertexData{
    Vector4 position; // 4D position vector
    Vector2 texcoord; // 2D texture coordinate vector
    Vector3 normal;
}; 