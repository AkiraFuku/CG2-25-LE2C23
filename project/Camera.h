#pragma once
#include "Vector4.h"
#include "WinApp.h"
class Camera
{
public:
    void Update();

private:
    Transform transform;
    Matrix4x4 worldMatrix;
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatirx;
    Matrix4x4 viewProtectionMatrix;
    float fovY=0.45f;
    float aspect=static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight);
    float nearCrip=0.1f;
    float farCrip=100.0f;


};

