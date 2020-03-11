#pragma once

#include "pch.h""

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

namespace Buffers
{
    template <typename T>
    class ConstantBuffer {
    public:
        ConstantBuffer() : BufferSize(sizeof(T) + (256 - sizeof(T) % 256))
        {
            ID3D12Device* device = g_deviceResources->GetD3DDevice();

            D3D12_HEAP_PROPERTIES uploadHeapProperties;
            uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
            uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            uploadHeapProperties.CreationNodeMask = 0;
            uploadHeapProperties.VisibleNodeMask = 0;
            
            DX::ThrowIfFailed(device->CreateCommittedResource(
                &uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(BufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(Buffer.GetAddressOf())
            ));
            Description = { Buffer->GetGPUVirtualAddress(), BufferSize };
            
            DX::ThrowIfFailed(Buffer->Map(0, nullptr, reinterpret_cast<void**>(&BufferMap)));
        };
        ConstantBuffer& operator=(const ConstantBuffer&) = default;

        Microsoft::WRL::ComPtr<ID3D12Resource> Buffer;
        UINT8* BufferMap;
        const UINT BufferSize;
        T Data;

        D3D12_CONSTANT_BUFFER_VIEW_DESC Description;

        void Write(T& data) {
            Data = data;

            ZeroMemory(BufferMap, BufferSize);
            memcpy(BufferMap, &Data, sizeof(Data));
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
        Vector3 eye;
    };

    typedef struct Environment {
        Vector3 light;
        float time;
    };

    typedef struct Material {
        Vector4 lightColor;
        Vector3 Ka;
        Vector3 Kd;
        Vector3 Ks;
        float alpha;
    };
}