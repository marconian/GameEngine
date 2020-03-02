#pragma once

#include "Sphere.h"
#include "Buffers.h"

class Planet
{
public:
    Planet(double mass, double size, std::vector<Planet>& planets) noexcept(false);
    Planet(const Planet& planet) :
        m_color(planet.m_color),
        //m_device(planet.m_device),
        m_descriptorHeap(planet.m_descriptorHeap),
        m_environment(planet.m_environment),
        m_graphicInfo(planet.m_graphicInfo),
        m_indexBuffer(planet.m_indexBuffer),
        m_indexBufferUpload(planet.m_indexBufferUpload),
        m_material(planet.m_material),
        m_mass(planet.m_mass),
        //m_mvp(planet.m_mvp),
        m_origin(planet.m_origin),
        m_position(planet.m_position),
        m_planets(planet.m_planets),
        m_pso(planet.m_pso),
        m_rootSignature(planet.m_rootSignature),
        m_rotation(planet.m_rotation),
        m_size(planet.m_size),
        m_velocity(planet.m_velocity),
        m_vertexBuffer(planet.m_vertexBuffer),
        m_vertexBufferUpload(planet.m_vertexBufferUpload) { }
    //Planet(const Planet& planet) = delete;
    Planet& operator=(const Planet&) = delete;
    
    ~Planet() { };

    void Render(ID3D12GraphicsCommandList* commandList);
    void WriteToGraphicBuffers(ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, D3D12_INDEX_BUFFER_VIEW& indexBufferView);
    void Update(DX::StepTimer const& timer);

    void Apply(DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

    void SetPositionAndVelocity(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Vector3 velocity) {
        m_position = position;
        m_origin = position;
        m_velocity = velocity;
    }
    void SetColor(DirectX::XMVECTORF32 color) { m_color = color; }
    //void SetParent(const Planet* parent) { m_parent = parent; }

    DirectX::SimpleMath::Vector3 GetPosition() { return m_position; }
    DirectX::SimpleMath::Vector3 GetVelocity() { return m_velocity; }
    double GetMass() { return m_mass; }
    double GetDiameter() { return m_size; }
    double GetSize() { return m_size * 10; }

    std::vector<DirectX::VertexPositionNormalColorTexture> GetVerticesInput(Sphere::Mesh& mesh)
    {
        std::vector<DirectX::VertexPositionNormalColorTexture> vertices;
        for (DirectX::SimpleMath::Vector3 p : mesh.vertices)
        {
            vertices.push_back(DirectX::VertexPositionNormalColorTexture(
                p * GetSize(), 
                -p * 5,
                m_color, 
                DirectX::SimpleMath::Vector2(p.x, p.y)
            ));
        }

        return vertices;
    }

    void CreateDeviceDependentResources();

private:
    Buffers::ConstantBuffer<Buffers::Environment>               m_environment;
    Buffers::ConstantBuffer<Buffers::Material>                  m_material;

    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_vertexBufferUpload;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_indexBufferUpload;

    Sphere::Mesh                                                m_graphicInfo;

    DirectX::SimpleMath::Vector3                                m_position;
    DirectX::SimpleMath::Vector3                                m_origin;
    DirectX::SimpleMath::Vector3                                m_velocity;
    DirectX::SimpleMath::Quaternion                             m_rotation;
    DirectX::XMVECTORF32                                        m_color;
    std::vector<Planet>&                                        m_planets;
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


