#pragma once

#include "Sphere.h"
#include "Buffers.h"
#include "StepTimer.h"

class Planet
{
public:
    Planet(double mass, double size, DirectX::XMVECTORF32 color = DirectX::Colors::White) noexcept(false);
    Planet(const Planet& planet) :
        m_color(planet.m_color),
        m_descriptorHeap(planet.m_descriptorHeap),
        m_environment(planet.m_environment),
        m_graphicInfo(planet.m_graphicInfo),
        m_indexBuffer(planet.m_indexBuffer),
        m_indexBufferUpload(planet.m_indexBufferUpload),
        m_instanceBuffer(planet.m_instanceBuffer),
        m_instanceBufferUpload(planet.m_instanceBufferUpload),
        m_material(planet.m_material),
        m_mass(planet.m_mass),
        m_origin(planet.m_origin),
        m_position(planet.m_position),
        m_pso(planet.m_pso),
        m_rootSignature(planet.m_rootSignature),
        m_rotation(planet.m_rotation),
        m_size(planet.m_size),
        m_velocity(planet.m_velocity),
        m_vertexBuffer(planet.m_vertexBuffer),
        m_vertexBufferUpload(planet.m_vertexBufferUpload),
        m_vertices(planet.m_vertices) { }
    Planet& operator=(const Planet& planet) = delete;
    
    ~Planet() { };

    void Render(ID3D12GraphicsCommandList* commandList);
    void WriteToGraphicBuffers(ID3D12GraphicsCommandList* commandList);
    void Update(DX::StepTimer const& timer);

    void SetPosition(const DirectX::SimpleMath::Vector3 position, const DirectX::SimpleMath::Vector3 direction, const float velocity) {
        m_position = position;
        m_origin = position;
        m_velocity = direction * velocity;
    }
    void SetColor(DirectX::XMVECTORF32 color) { m_color = color; }

    DirectX::SimpleMath::Vector3 GetPosition() { return m_position; }
    DirectX::SimpleMath::Vector3 GetVelocity() { return m_velocity; }
    double GetMass() { return m_mass; }
    double GetDiameter() { return m_size; }
    double GetSize() { return m_size; }

    const void UpdateVerticesInput(Sphere::Mesh& mesh);
    const Vector3 GetGravitationalAcceleration(Planet& planet);

    void CreateDeviceDependentResources();

private:
    Buffers::ConstantBuffer<Buffers::Environment>               m_environment;
    Buffers::ConstantBuffer<Buffers::Material>                  m_material;

    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_instanceBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_vertexBufferUpload;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_instanceBufferUpload;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_indexBufferUpload;

    Sphere::Mesh                                                m_graphicInfo;
    std::vector<DirectX::VertexPositionNormalColorTexture>      m_vertices;

    DirectX::SimpleMath::Vector3                                m_position;
    DirectX::SimpleMath::Vector3                                m_origin;
    DirectX::SimpleMath::Vector3                                m_velocity;
    DirectX::SimpleMath::Quaternion                             m_rotation;
    DirectX::XMVECTORF32                                        m_color;
    double                                                      m_size;
    double                                                      m_mass;

    Microsoft::WRL::ComPtr<ID3D12PipelineState>                 m_pso;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>                 m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>                m_descriptorHeap;

    // Create root signature.
    enum RootParameterIndex
    {
        ConstantBuffer0,
        ConstantBuffer1,
        ConstantBuffer2,
        TextureSRV,
        TextureSampler,
        RootParameterCount
    };
};


