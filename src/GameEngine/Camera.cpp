#include "pch.h"
#include "Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera(void) :
    mAngle(XM_PI / 4.f),
    mClientHeight(1),
    mClientWidth(1),
    mNearest(.1f),
    mFarthest(100.f),
    mPosition(0, 0, -1),
    mTarget(0, 0, 0),
    mUp(0, 1, -1)
{
    XMStoreFloat4x4(&mView, XMMatrixIdentity());
    XMStoreFloat4x4(&mProj, XMMatrixIdentity());
    XMStoreFloat4x4(&mOrtho, XMMatrixIdentity());

    this->InitViewMatrix();
}

Camera::Camera(const Camera& camera) { *this = camera; }

Camera& Camera::operator=(const Camera& camera)
{
    mPosition = camera.mPosition;
    mTarget = camera.mTarget;
    mUp = camera.mUp;
    mAngle = camera.mAngle;
    mClientWidth = camera.mClientWidth;
    mClientHeight = camera.mClientHeight;
    mNearest = camera.mNearest;
    mFarthest = camera.mFarthest;
    mView = camera.mView;
    mProj = camera.mProj;
    mOrtho = camera.mOrtho;

    return *this;
}

void Camera::InitViewMatrix()
{
    XMStoreFloat4x4(&mView, XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mTarget), XMLoadFloat3(&this->Up())));
}

void Camera::InitProjMatrix(const float angle, const float client_width, const float client_height,
    const float near_plane, const float far_plane)
{
    mAngle = angle;
    mClientWidth = client_width;
    mClientHeight = client_height;
    mNearest = near_plane;
    mFarthest = far_plane;

    XMStoreFloat4x4(&mProj, XMMatrixPerspectiveFovLH(angle, client_width / client_height, near_plane, far_plane));
}

void Camera::Move(Vector3 direction)
{
    mPosition = XMVector3Transform(mPosition, XMMatrixTranslation(direction.x, direction.y, direction.z));
    mTarget = XMVector3Transform(mTarget, XMMatrixTranslation(direction.x, direction.y, direction.z));
    mUp = XMVector3Transform(mUp, XMMatrixTranslation(direction.x, direction.y, direction.z));

    this->InitViewMatrix();
}

void Camera::Rotate(Vector3 axis, float degrees)
{
    if (XMVector3Equal(axis, XMVectorZero())) return;

    // rotate vectors 
    Vector3 look_at_target = mTarget - mPosition;
    Vector3 look_at_up = mUp - mPosition;

    look_at_target = XMVector3Transform(look_at_target,
        XMMatrixRotationAxis(axis, XMConvertToRadians(degrees)));
    look_at_up = XMVector3Transform(look_at_up, XMMatrixRotationAxis(axis,
        XMConvertToRadians(degrees)));

    // restore vectors's end points mTarget and mUp from new rotated vectors 
    mTarget = mPosition + look_at_target;
    mUp = mPosition + look_at_up; 
    
    this->InitViewMatrix();
}

void Camera::Target(Vector3 new_target)
{
    //if (XMVector3Equal(new_target, mPosition) || XMVector3Equal(new_target, mTarget)) return;

    Vector3 old_look_at_target = mTarget - mPosition;
    Vector3 new_look_at_target = new_target - mPosition;

    float angle = XMConvertToDegrees(XMVectorGetX(XMVector3AngleBetweenNormals(XMVector3Normalize(old_look_at_target),
        XMVector3Normalize(new_look_at_target))));

    XMVECTOR axis = XMVector3Cross(old_look_at_target, new_look_at_target);
    Rotate(axis, angle);

    //if (angle != 0.0f && angle != 360.0f && angle != 180.0f)
    //{
    //    XMVECTOR axis = XMVector3Cross(old_look_at_target, new_look_at_target);
    //    Rotate(axis, angle);
    //}

    mTarget = new_target;
    this->InitViewMatrix();
}

// Set camera position
void Camera::Position(Vector3& new_position)
{
    Vector3 move_vector = new_position - mPosition;
    Vector3 target = mTarget; 
    
    this->Move(move_vector);
    this->Target(target);
}

void Camera::Angle(float angle)
{
    mAngle = angle;
    InitProjMatrix(mAngle, mClientWidth, mClientHeight, mNearest, mFarthest);
}

void Camera::NearestPlane(float nearest)
{
    mNearest = nearest; 
    OnResize(mClientWidth, mClientHeight);
}

void Camera::FarthestPlane(float farthest)
{
    mFarthest = farthest;
    OnResize(mClientWidth, mClientHeight);
}

void Camera::InitOrthoMatrix(const float clientWidth, const float clientHeight, const float nearZ, const float fartherZ)
{
    XMStoreFloat4x4(&mOrtho, XMMatrixOrthographicLH(clientWidth, clientHeight, 0.0f, fartherZ));
}

void Camera::OnResize(uint32_t new_width, uint32_t new_height)
{
    mClientWidth = new_width; 
    mClientHeight = new_height;

    InitProjMatrix(mAngle, static_cast<float>(new_width), static_cast<float>(new_height), mNearest, mFarthest);
    InitOrthoMatrix(static_cast<float>(new_width), static_cast<float>(new_height), 0.0f, mFarthest);
}