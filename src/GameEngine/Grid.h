#pragma once

class Grid
{
public:
	Grid() noexcept(false);

	~Grid()
	{
	};

	void Render(ID3D12GraphicsCommandList* commandList);
	void Update(DX::StepTimer const& timer);

	void SetOrigin(DirectX::SimpleMath::Vector3 origin) { m_origin = origin; }
	void SetSize(float size) { m_size = size; }
	void SetCellSize(float size) { m_cellsize = size; }

	void SetColor(DirectX::XMVECTORF32 color) { m_color = color; }

	void CreateDeviceDependentResources();

private:
	struct Line
	{
		Line(): p1(), p2()
		{
		}

		Line(DirectX::VertexPositionColor p1, DirectX::VertexPositionColor p2) :
			p1(p1), p2(p2)
		{
		}

		DirectX::VertexPositionColor p1;
		DirectX::VertexPositionColor p2;
	};

	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
	std::unique_ptr<DirectX::BasicEffect> m_effect;

	DirectX::SimpleMath::Vector3 m_origin;
	DirectX::SimpleMath::Quaternion m_rotation;
	float m_size;
	float m_cellsize;
	DirectX::XMVECTORF32 m_color;

	std::vector<Line> m_lines;
};
