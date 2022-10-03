#include "pch.h"
#pragma once

#include "Globals.h"
#include "Cluster.h"
using std::fpclassify;

#include <cmath>
#include <vector>
#include <vector>


Cluster::Cluster(const BoundingBox bbox, std::vector<const Planet*> planets, const float cluster_size): m_bbox(bbox), m_mass(0), m_no_planets(0)
{
	for (const auto& planet : planets)
		m_mass += planet->GetMass();

	m_no_planets = planets.size();
	if (planets.size() > 1)
	{
		const float step_x = (bbox.xmax - bbox.xmin) / cluster_size;
		const float step_y = (bbox.ymax - bbox.ymin) / cluster_size;
		const float step_z = (bbox.zmax - bbox.zmin) / cluster_size;
		constexpr float epsilon = 1e-7f;

		if (fpclassify(step_x) == FP_SUBNORMAL || fpclassify(step_y) == FP_SUBNORMAL || fpclassify(step_z) == FP_SUBNORMAL || 
			step_x < epsilon || step_y < epsilon || step_z < epsilon)
		{
			return;
		}

		for (float x = bbox.xmin; x < bbox.xmax; x += step_x) {
			for (float y = bbox.ymin; y < bbox.ymax; y += step_y) {
				for (float z = bbox.zmin; z < bbox.zmax; z += step_z) {
					std::vector<const Planet*> in_cluster{};
					for (int i = planets.size() - 1; i >= 0; i--)
					{
						const auto& planet = *planets[i];
						if (auto& position = planet.GetPosition(); position.x >= x && position.x < x + step_x && 
							position.y >= y && position.y < y + step_y &&
							position.z >= z && position.z < z + step_z)
						{
							in_cluster.push_back(&planet);
							planets.erase(planets.begin() + i);
						}
					}
					planets.shrink_to_fit();

					if (!in_cluster.empty())
					{
						m_clusters.push_back(Cluster(
							{ x, y, z, x + step_x, y + step_y, z + step_z }, 
							in_cluster));
					}
				}
			}
		}
	}
}

std::vector<Grid::Line> Cluster::GetGridLines(DirectX::XMVECTORF32& color) const
{
	std::vector<Grid::Line> lines{};
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmin, m_bbox.ymin, m_bbox.zmin}, color},
		{{m_bbox.xmin, m_bbox.ymax, m_bbox.zmin}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmin, m_bbox.ymax, m_bbox.zmin}, color},
		{{m_bbox.xmax, m_bbox.ymax, m_bbox.zmin}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmax, m_bbox.ymax, m_bbox.zmin}, color},
		{{m_bbox.xmax, m_bbox.ymin, m_bbox.zmin}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmax, m_bbox.ymin, m_bbox.zmin}, color},
		{{m_bbox.xmin, m_bbox.ymin, m_bbox.zmin}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmin, m_bbox.ymin, m_bbox.zmax}, color},
		{{m_bbox.xmin, m_bbox.ymax, m_bbox.zmax}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmin, m_bbox.ymax, m_bbox.zmax}, color},
		{{m_bbox.xmax, m_bbox.ymax, m_bbox.zmax}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmax, m_bbox.ymax, m_bbox.zmax}, color},
		{{m_bbox.xmax, m_bbox.ymin, m_bbox.zmax}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmax, m_bbox.ymin, m_bbox.zmax}, color},
		{{m_bbox.xmin, m_bbox.ymin, m_bbox.zmax}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmin, m_bbox.ymin, m_bbox.zmin}, color},
		{{m_bbox.xmin, m_bbox.ymin, m_bbox.zmax}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmin, m_bbox.ymax, m_bbox.zmin}, color},
		{{m_bbox.xmin, m_bbox.ymax, m_bbox.zmax}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmax, m_bbox.ymax, m_bbox.zmin}, color},
		{{m_bbox.xmax, m_bbox.ymax, m_bbox.zmax}, color}
	));
	lines.emplace_back(Grid::Line(
		{{m_bbox.xmax, m_bbox.ymin, m_bbox.zmin}, color},
		{{m_bbox.xmax, m_bbox.ymin, m_bbox.zmax}, color}
	));
	for (auto& subcluster : m_clusters)
	{
		for (auto& line : subcluster.GetGridLines(color))
			lines.emplace_back(line);
	}
	return lines;
}

float Cluster::size() const
{
	const float size_x = m_bbox.xmax - m_bbox.xmin;
	const float size_y = m_bbox.ymax - m_bbox.ymin;
	const float size_z = m_bbox.zmax - m_bbox.zmin;
	return size_x * size_y * size_z;
}
