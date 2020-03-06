#pragma once
//////////////////////////////////////////////////////// 
// Stores View and Projection matrices used by shaders 
// to translate 3D world into 2D screen surface 
// Camera can be moved and rotated. Also, user can change  
// camera's target and position 
////////////////////////////////////////////////////////

class Camera
{
public:

    // Constructs default camera looking at 0,0,0 
    // placed at 0,0,-1 with up vector 0,1,0 (note that mUp is NOT a vector - it's vector's end) 
    Camera(void); // Create camera, based on another one 
    Camera(const Camera& camera); // Copy all camera's parameters 
    Camera& operator=(const Camera& camera);
    ~Camera(void) {}

private:

    // Initialize camera's View matrix from mPosition, mTarget and mUp coordinates 
    void InitViewMatrix();

public:

    // Initialize camera's perspective Projection matrix 
    void InitProjMatrix(const float angle, const float client_width, const float client_height, const float nearest, const float farthest);

    // Initialize camera's orthogonal projection 
    void InitOrthoMatrix(const float client_width, const float client_height, const float near_plane, const float far_plane);

    // Resize matrices when window size changes 
    void OnResize(uint32_t new_width, uint32_t new_height);

    /////////////////////////////////////////////// 
    /*** View matrix transformation interfaces ***/
    /////////////////////////////////////////////// 
    // Move camera 
    void Move(DirectX::SimpleMath::Vector3 direction); 
    // Rotate camera around `axis` by `degrees`. Camera's position is a  
    // pivot point of rotation, so it doesn't change 
    void Rotate(DirectX::SimpleMath::Vector3 axis, float degrees);

    // Set camera position coordinates
    void Position(DirectX::SimpleMath::Vector3& new_position);

    // Get camera position coordinates 
    const DirectX::SimpleMath::Vector3& Position() const { return mPosition; }

    // Change camera target position 
    void Target(DirectX::SimpleMath::Vector3 new_target);

    // Get camera's target position coordinates 
    const DirectX::SimpleMath::Vector3& Target() const { return mTarget; }

    // Get camera's up vector 
    const DirectX::SimpleMath::Vector3 Up() { return DirectX::SimpleMath::Vector3::Up; }

    // Get camera's look at target 
    const DirectX::SimpleMath::Vector3 LookAtTarget() { return mTarget - mPosition; }

    ///////////////////////////////////////////////////// 
    /*** Projection matrix transformation interfaces ***/
    ///////////////////////////////////////////////////// 
    // Set view frustum's angle 
    void Angle(float angle); 
    // Get view frustum's angle 
    const float& Angle() const { return mAngle; }

    // Set nearest culling plane distance from view frustum's projection plane 
    void NearestPlane(float nearest);

    // Set farthest culling plane distance from view frustum's projection plane 
    void FarthestPlane(float farthest);

    // Returns transposed camera's View matrix  
    const DirectX::SimpleMath::Matrix View() { return mView; }

    // Returns transposed camera's Projection matrix 
    const DirectX::SimpleMath::Matrix Proj() { return mProj; }

    // Returns transposed orthogonal camera matrix 
    const DirectX::SimpleMath::Matrix Ortho() { return mOrtho; }

private:

    /*** Camera parameters ***/
    DirectX::SimpleMath::Vector3 mPosition;

    // Camera's coordinates 
    DirectX::SimpleMath::Vector3 mTarget;

    // Camera's up vector end coordinates 

    /*** Projection parameters ***/
    float mAngle; // Angle of view frustum 
    float mClientWidth; // Window's width 
    float mClientHeight;    // Window's height 
    float mNearest; // Nearest view frustum plane 
    float mFarthest; // Farthest view frustum plane 

    DirectX::SimpleMath::Matrix mView; // View matrix 
    DirectX::SimpleMath::Matrix  mProj; // Projection matrix 
    DirectX::SimpleMath::Matrix  mOrtho; // Ortho matrix for drawing without tranformation 
};
