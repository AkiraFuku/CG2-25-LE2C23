#include "Camera.h"
#include "MathFunction.h"
#include "ImGuiManager.h"
Camera::Camera()
    :transform_({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} })
    , fovY(0.45f)
    , aspect(static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight))
    , nearCrip(0.1f)
    , farCrip(100.0f)
    , worldMatrix(MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate))
    , viewMatrix(Inverse(worldMatrix))
    , projectionMatirx(MakePerspectiveFovMatrix(fovY, aspect, nearCrip, farCrip))
    , viewProtectionMatrix(Multiply(viewMatrix, projectionMatirx))
{}
void Camera::Update() {

#ifdef USE_IMGUI

    ImGui::Begin("Camera");
    ImGui::DragFloat3("translate", &(transform_.translate.x),0.1f);
    ImGui::SliderAngle("rotateX", &(transform_.rotate.x));
    ImGui::SliderAngle("rotateY", &(transform_.rotate.y));
    ImGui::SliderAngle("rotateZ", &(transform_.rotate.z));
    ImGui::End();

#endif // USE_IMGUI


    worldMatrix = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    viewMatrix = Inverse(worldMatrix);
    projectionMatirx = MakePerspectiveFovMatrix(fovY, aspect, nearCrip, farCrip);

    viewProtectionMatrix = Multiply(viewMatrix, projectionMatirx);

}