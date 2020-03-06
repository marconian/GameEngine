#pragma once

#include <map>

class Sphere
{
public:
	struct Mesh
	{
		std::vector<DirectX::SimpleMath::Vector3> vertices;
		std::vector<uint32_t> indices;

		uint32_t triangleCount() const;
		void addTriangle(uint32_t a, uint32_t b, uint32_t c);
		void addQuad(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
		void addQuadAlt(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
		void clear();
		double distance(const DirectX::SimpleMath::Vector3& p, uint32_t tidx) const;
		double distance(const DirectX::SimpleMath::Vector3& p) const;
	};

	static Mesh Create(int lod)
	{
		Mesh sphereDesc;
		Icosahedron(sphereDesc);

		Mesh temp = sphereDesc;
		for (int i = 0; i < lod; i++)
		{
			SubdivideMesh(temp, sphereDesc);
			temp = sphereDesc;
		}

		return sphereDesc;
	}
private:
	static double dot(const DirectX::SimpleMath::Vector3& a, const DirectX::SimpleMath::Vector3& b);
	static DirectX::SimpleMath::Vector3 cross(const DirectX::SimpleMath::Vector3& a, const DirectX::SimpleMath::Vector3& b);
	static double length(const DirectX::SimpleMath::Vector3& a);
	static DirectX::SimpleMath::Vector3 normalize(const DirectX::SimpleMath::Vector3& a);

	struct Edge
	{
		uint32_t v0;
		uint32_t v1;

		Edge(uint32_t v0, uint32_t v1) :
			v0(v0 < v1 ? v0 : v1),
			v1(v0 < v1 ? v1 : v0) {}

		bool operator <(const Edge& rhs) const
		{
			return v0 < rhs.v0 || (v0 == rhs.v0 && v1 < rhs.v1);
		}
	};

	static uint32_t SubdivideEdge(uint32_t f0, uint32_t f1, const DirectX::SimpleMath::Vector3& v0, const DirectX::SimpleMath::Vector3& v1, Mesh& io_mesh, std::map<Edge, uint32_t>& io_divisions);
	static void SubdivideMesh(const Mesh& meshIn, Mesh& meshOut);
	static void Icosahedron(Mesh& mesh);
};



