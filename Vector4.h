#pragma once
struct Vector4{
    float x;
    float y;
    float z;
    float w;
};
struct Vector2{
    float x;
    float y;
};
struct Matrix4x4{
   float m[4][4];
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
    Vector4 position;
    Vector2 texcoord;

};