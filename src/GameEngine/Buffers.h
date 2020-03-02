#pragma once

#include "pch.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

namespace Buffers
{
    template <typename T>
    class ConstantBuffer {
    public:
        ConstantBuffer(ID3D12Device* device) : BufferSize(sizeof(T) + (256 - sizeof(T) % 256))
        {
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

    typedef struct ModelViewProjection {
        Matrix mv;
        Matrix mvp;
    };

    typedef struct Environment {
        Vector3 position;
        Vector4 light;
    };

    typedef struct Material {
        Vector4 lightColor;
        Vector4 Ka;
        Vector4 Kd;
        Vector4 Ks;
        Vector4 shininess;
    };
}