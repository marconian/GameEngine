#pragma once

#include "pch.h""

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

namespace Buffers
{
    template <typename T, size_t S = 1>
    class ConstantBuffer {
    public:
        ConstantBuffer() : BufferSize((sizeof(T) * S + 0xff) & ~0xff)
        {
            ID3D12Device* device = g_deviceResources->GetD3DDevice();

            //D3D12_HEAP_PROPERTIES uploadHeapProperties;
            //uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
            //uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            //uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            //uploadHeapProperties.CreationNodeMask = 0;
            //uploadHeapProperties.VisibleNodeMask = 0;

            CD3DX12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer((UINT64)BufferSize);
            //rd.Width = sizeof(T); // (sizeof(T) * m_size + 0xff) & ~0xff;
            //rd.DepthOrArraySize = S;
            
            DX::ThrowIfFailed(device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &rd,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(Buffer.GetAddressOf())
            ));
            Description = { Buffer->GetGPUVirtualAddress(), BufferSize };

            //CD3DX12_RANGE readRange(0, 0);
            //DX::ThrowIfFailed(Buffer->Map(0, &readRange, reinterpret_cast<void**>(&BufferMap)));
        };
        ConstantBuffer& operator=(const ConstantBuffer&) = default;

        Microsoft::WRL::ComPtr<ID3D12Resource> Buffer;
        T* BufferMap;
        const UINT BufferSize;

        D3D12_CONSTANT_BUFFER_VIEW_DESC Description;

        void Write(T* data) {
            CD3DX12_RANGE readRange(0, BufferSize);
            void** addr = reinterpret_cast<void**>(&BufferMap);
            DX::ThrowIfFailed(Buffer->Map(0, &readRange, addr));

            ZeroMemory(BufferMap, BufferSize);
            memcpy(BufferMap, data, sizeof(T) * S);

            Buffer->Unmap(0, &readRange);
        }
    };

    struct XMDOUBLE3 {
        double x;
        double y;
        double z;

        XMDOUBLE3() = default;

        XMDOUBLE3(const XMDOUBLE3&) = default;
        XMDOUBLE3& operator=(const XMDOUBLE3&) = default;

        XMDOUBLE3(XMDOUBLE3&&) = default;
        XMDOUBLE3& operator=(XMDOUBLE3&&) = default;

        XM_CONSTEXPR XMDOUBLE3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
        explicit XMDOUBLE3(_In_reads_(3) const double* pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]) {}
        XM_CONSTEXPR XMDOUBLE3(const XMFLOAT3 _v) : x(_v.x), y(_v.y), z(_v.z) {}
    };

    struct XMDOUBLE4 {
        double x;
        double y;
        double z;
        double w;

        XMDOUBLE4() = default;

        XMDOUBLE4(const XMDOUBLE4&) = default;
        XMDOUBLE4& operator=(const XMDOUBLE4&) = default;

        XMDOUBLE4(XMDOUBLE4&&) = default;
        XMDOUBLE4& operator=(XMDOUBLE4&&) = default;

        XM_CONSTEXPR XMDOUBLE4(double _x, double _y, double _z, double _w) : x(_x), y(_y), z(_z), w(_w) {}
        explicit XMDOUBLE4(_In_reads_(4) const double* pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}
        XM_CONSTEXPR XMDOUBLE4(const XMFLOAT4 _v) : x(_v.x), y(_v.y), z(_v.z), w(_v.w) {}
    };

    typedef struct ModelViewProjection {
        Matrix m;
        Matrix v;
        Matrix p;
        Matrix mv;
        Matrix mp;
        Matrix vp;
        Matrix mvp;
        XMFLOAT3 eye;
    };

    typedef struct Settings {
        uint32_t coreView;
    };

    typedef struct Environment {
        float deltaTime;
        float totalTime;
        XMFLOAT3 light;
    };

    typedef struct System {
        float systemMass;
        XMFLOAT3 centerOfMass;
    };

    typedef struct Material {
        XMFLOAT4 lightColor;
        XMFLOAT3 Ka;
        XMFLOAT3 Kd;
        XMFLOAT3 Ks;
        float alpha;
    };
}