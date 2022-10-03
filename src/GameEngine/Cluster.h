#pragma once
#include "Grid.h"

class Cluster
{

public:
	struct BoundingBox
	{
		float xmin;
		float ymin;
		float zmin;
		float xmax;
		float ymax;
		float zmax;
	};

	Cluster() : m_mass(0), m_no_planets(0) {}
	Cluster(const BoundingBox bbox, std::vector<const Planet*> planets, const float cluster_size = 2);

	~Cluster() = default;

	std::vector<Grid::Line> GetGridLines(DirectX::XMVECTORF32& color) const;
	[[nodiscard]] float size() const;

	const BoundingBox m_bbox{};
	float m_mass;
	size_t m_no_planets;
	std::vector<Cluster> m_clusters{};

};

