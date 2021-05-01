#pragma once

#include <map>

class Sphere
{
public:
	struct Mesh
	{
		std::vector<DirectX::SimpleMath::Vector3> vertices;
		std::vector<uint32_t> indices;
		std::vector<uint32_t> indices_p;

		[[nodiscard]] uint32_t triangle_count() const;
		void add_triangle(uint32_t a, uint32_t b, uint32_t c);
		void add_quad(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
		void add_quad_alt(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
		void clear();
		[[nodiscard]] double distance(const DirectX::SimpleMath::Vector3& p, uint32_t tidx) const;
		[[nodiscard]] double distance(const DirectX::SimpleMath::Vector3& p) const;
	};

	static Mesh create(const int lod)
	{
		auto sphere_desc = icosahedron();

		for (int i = 0; i < lod; i++)
			subdivide_mesh(sphere_desc);

		return sphere_desc;
	}

private:
	static double dot(const DirectX::SimpleMath::Vector3& a, const DirectX::SimpleMath::Vector3& b);
	static DirectX::SimpleMath::Vector3 cross(const DirectX::SimpleMath::Vector3& a,
	                                          const DirectX::SimpleMath::Vector3& b);
	static double length(const DirectX::SimpleMath::Vector3& a);
	static DirectX::SimpleMath::Vector3 normalize(const DirectX::SimpleMath::Vector3& a);

	struct edge
	{
		uint32_t v0;
		uint32_t v1;

		edge(uint32_t v0, uint32_t v1) :
			v0(v0 < v1 ? v0 : v1),
			v1(v0 < v1 ? v1 : v0)
		{
		}

		bool operator <(const edge& rhs) const
		{
			return v0 < rhs.v0 || (v0 == rhs.v0 && v1 < rhs.v1);
		}
	};

	static Mesh icosahedron();

	static void subdivide_mesh(Mesh& mesh);
	static uint32_t subdivide_edge(uint32_t f0, uint32_t f1, const DirectX::SimpleMath::Vector3& v0,
	                               const DirectX::SimpleMath::Vector3& v1, Mesh& io_mesh,
	                               std::map<edge, uint32_t>& io_divisions);
};
