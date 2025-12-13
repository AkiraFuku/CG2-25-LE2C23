#include "Camera.h"
#include "MassFunction.h"
void Camera::Update(){

    worldMatrix=MakeAfineMatrix(transform.scale,transform.rotate,transform.translate);
    viewMatrix=Inverse(worldMatrix);
    projectionMatirx = MakePerspectiveFovMatrix(fovY,aspect,nearCrip ,farCrip);

    viewProtectionMatrix=Multiply(viewMatrix,projectionMatirx)

}