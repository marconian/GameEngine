#include "pch.h"
#include "Terrain.h"
#include "Sphere.h"
#include "ShaderTools.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Terrain::Terrain(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, bool msaa, int sampleCount) :
    m_device(device),
    m_backBufferFormat(backBufferFormat),
    m_depthBufferFormat(depthBufferFormat),
    m_msaa(msaa),
    m_sampleCount(sampleCount),
    m_origin(Vector3::Zero),
    m_rotation(Quaternion::Identity),
    m_divisions(20),
    m_cellsize(2.f),
    m_color(Colors::White)
{ 
    CreateDeviceDependentResources();
}

void Terrain::Render(ID3D12GraphicsCommandList* commandList)
{
    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);
    m_effect->SetWorld(m_world);
    m_effect->SetAmbientLightColor(Colors::White);
    m_effect->EnableDefaultLighting();
    m_effect->SetLightEnabled(0, true);
    m_effect->SetLightDirection(0, Vector3::Up);

    m_effect->Apply(commandList);

    int detail = 2;

    Mesh sphereDesc;
    Icosahedron(sphereDesc);

    Mesh temp = sphereDesc;
    for (int i = 0; i < detail; i++)
    {
        SubdivideMesh(temp, sphereDesc);
        temp = sphereDesc;
    }

    m_batch->Begin(commandList);

    int t_max = sphereDesc.triangles.size();
    for (int i = 0; i < t_max; i += 3)
    {
        m_batch->DrawTriangle(
            VertexPositionNormalColor(sphereDesc.vertices[sphereDesc.triangles[i]], Vector3::One, m_color),
            VertexPositionNormalColor(sphereDesc.vertices[sphereDesc.triangles[i + 1]], Vector3::One, m_color),
            VertexPositionNormalColor(sphereDesc.vertices[sphereDesc.triangles[i + 2]], Vector3::One, m_color)
        );
    }

    m_batch->End();
}

void Terrain::Update()
{

}

void Terrain::CreateDeviceDependentResources()
{
    RenderTargetState rtState(m_backBufferFormat, m_depthBufferFormat);

    CD3DX12_RASTERIZER_DESC rastDesc;
    if (m_msaa)
    {
        rtState.sampleDesc.Count = m_sampleCount;
        rastDesc = CD3DX12_RASTERIZER_DESC(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, FALSE,
            D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
            D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE,
            0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
    }

    EffectPipelineStateDescription pd(
        &VertexPositionNormalColor::InputLayout,
        CommonStates::Opaque,
        CommonStates::DepthDefault,
        rastDesc,
        rtState,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    //D3D12_SHADER_BYTECODE vertexShader;
    //GetShader("SphereVertexShader", vertexShader);

    //D3D12_SHADER_BYTECODE pixelShader;
    //GetShader("SpherePixelShader", pixelShader);

    //{
    //    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
    //        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
    //        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
    //        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
    //        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    //    // Create root parameters and initialize first (constants)
    //    CD3DX12_ROOT_PARAMETER rootParameters[RootParameterIndex::RootParameterCount] = {};
    //    rootParameters[RootParameterIndex::ConstantBuffer].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

    //    // Root parameter descriptor
    //    CD3DX12_ROOT_SIGNATURE_DESC rsigDesc = {};

    //    // Include texture and srv
    //    CD3DX12_DESCRIPTOR_RANGE textureSRV(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    //    CD3DX12_DESCRIPTOR_RANGE textureSampler(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

    //    rootParameters[RootParameterIndex::TextureSRV].InitAsDescriptorTable(1, &textureSRV, D3D12_SHADER_VISIBILITY_PIXEL);
    //    rootParameters[RootParameterIndex::TextureSampler].InitAsDescriptorTable(1, &textureSampler, D3D12_SHADER_VISIBILITY_PIXEL);

    //    // use all parameters
    //    rsigDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    //    DX::ThrowIfFailed(CreateRootSignature(m_device, &rsigDesc, m_rootSignature.GetAddressOf()));
    //}

    //ComPtr<ID3D12PipelineState> pso;
    //pd.CreatePipelineState(
    //    m_device,
    //    m_rootSignature.Get(),
    //    vertexShader,
    //    pixelShader,
    //    pso.GetAddressOf()
    //);

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionNormalColor>>(m_device);
    m_effect = std::make_unique<BasicEffect>(m_device, EffectFlags::VertexColor, pd);
}