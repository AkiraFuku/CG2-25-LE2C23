#include "Camera.h"
#include "MathFunction.h"
#include "imgui.h"
Camera::Camera()
    :transform_({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} })
    , fovY(0.45f)
    , aspect(static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight))
    , nearCrip(0.1f)
    , farCrip(100.0f)
    , worldMatrix(MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate))
    , viewMatrix(Inverse(worldMatrix))
    , projectionMatrix(MakePerspectiveFovMatrix(fovY, aspect, nearCrip, farCrip))
    , viewProtectionMatrix(Multiply(viewMatrix, projectionMatrix))
{}
void Camera::Update() {

#ifdef USE_IMGUI

        ImGui::Begin("camera");
         ImGui::DragFloat3("Camera Pos", & transform_.translate.x, 0.1f);
        ImGui::End();


#endif // USE_IMGUI



    worldMatrix = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    viewMatrix = Inverse(worldMatrix);
    projectionMatrix = MakePerspectiveFovMatrix(fovY, aspect, nearCrip, farCrip);

    viewProtectionMatrix = Multiply(viewMatrix, projectionMatrix);

}