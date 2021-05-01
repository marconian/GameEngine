#pragma once

#include "pch.h"
#include "Globals.h"

using Microsoft::WRL::ComPtr;

namespace Buffers
{
	template <typename T, size_t S = 1>
	class ConstantBuffer
	{
	public:
		ConstantBuffer() : BufferMap(nullptr), BufferSize((sizeof(T) * S + 0xff) & ~0xff)
		{
			ID3D12Device* device = g_device_resources->GetD3DDevice();

			auto rd = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(BufferSize));
			auto hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

			DX::ThrowIfFailed(device->CreateCommittedResource(
				&hp,
				D3D12_HEAP_FLAG_NONE,
				&rd,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(Buffer.GetAddressOf())
			));
			Description = {Buffer->GetGPUVirtualAddress(), BufferSize};

			//CD3DX12_RANGE readRange(0, 0);
			//DX::ThrowIfFailed(Buffer->Map(0, &readRange, reinterpret_cast<void**>(&BufferMap)));
		};
		ConstantBuffer& operator=(const ConstantBuffer&) = default;

		ComPtr<ID3D12Resource> Buffer;
		T* BufferMap;
		const UINT BufferSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC Description;

		void Write(T* data)
		{
			CD3DX12_RANGE readRange(0, BufferSize);
			void** addr = reinterpret_cast<void**>(&BufferMap);
			DX::ThrowIfFailed(Buffer->Map(0, &readRange, addr));

			ZeroMemory(BufferMap, BufferSize);
			memcpy(BufferMap, data, sizeof(T) * S);

			Buffer->Unmap(0, &readRange);
		}
	};

	struct XMDOUBLE3
	{
		double x;
		double y;
		double z;

		XMDOUBLE3() = default;

		XMDOUBLE3(const XMDOUBLE3&) = default;
		XMDOUBLE3& operator=(const XMDOUBLE3&) = default;

		XMDOUBLE3(XMDOUBLE3&&) = default;
		XMDOUBLE3& operator=(XMDOUBLE3&&) = default;

		XM_CONSTEXPR XMDOUBLE3(const double x, const double y, const double z) : x(x), y(y), z(z)
		{
		}

		explicit XMDOUBLE3(_In_reads_(3) const double* p_array) : x(p_array[0]), y(p_array[1]), z(p_array[2])
		{
		}

		XM_CONSTEXPR XMDOUBLE3(const DirectX::XMFLOAT3 v) : x(static_cast<double>(v.x)), y(static_cast<double>(v.y)), z(
			                                                    static_cast<double>(v.z))
		{
		}
	};

	struct XMDOUBLE4
	{
		double x;
		double y;
		double z;
		double w;

		XMDOUBLE4() = default;

		XMDOUBLE4(const XMDOUBLE4&) = default;
		XMDOUBLE4& operator=(const XMDOUBLE4&) = default;

		XMDOUBLE4(XMDOUBLE4&&) = default;
		XMDOUBLE4& operator=(XMDOUBLE4&&) = default;

		XM_CONSTEXPR XMDOUBLE4(double _x, double _y, double _z, double _w) : x(_x), y(_y), z(_z), w(_w)
		{
		}

		explicit XMDOUBLE4(_In_reads_(4) const double* pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3])
		{
		}

		XM_CONSTEXPR XMDOUBLE4(const DirectX::XMFLOAT4 _v) : x(static_cast<double>(_v.x)), y(static_cast<double>(_v.y)),
		                                                     z(
			                                                     static_cast<double>(_v.z)),
		                                                     w(static_cast<double>(_v.w))
		{
		}
	};

	struct ModelViewProjection
	{
		DirectX::SimpleMath::Matrix m;
		DirectX::SimpleMath::Matrix v;
		DirectX::SimpleMath::Matrix p;
		DirectX::SimpleMath::Matrix mv;
		DirectX::SimpleMath::Matrix mp;
		DirectX::SimpleMath::Matrix vp;
		DirectX::SimpleMath::Matrix mvp;
		DirectX::XMFLOAT3 eye;
	};

	struct Settings
	{
		uint32_t coreView;
	};

	struct Environment
	{
		float deltaTime;
		float totalTime;
		DirectX::XMFLOAT3 light;
	};

	struct System
	{
		float systemMass;
		DirectX::XMFLOAT3 centerOfMass;
	};

	struct Material
	{
		DirectX::XMFLOAT4 lightColor;
		DirectX::XMFLOAT3 Ka;
		DirectX::XMFLOAT3 Kd;
		DirectX::XMFLOAT3 Ks;
		float alpha;
	};
}
