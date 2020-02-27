#pragma once

#include <map>

class Sphere
{
public:
	struct Mesh
	{
		std::vector<DirectX::SimpleMath::Vector3> vertices;
		std::vector<uint16_t> triangles;

		uint16_t triangleCount() const;
		void addTriangle(uint16_t a, uint16_t b, uint16_t c);
		void addQuad(uint16_t a, uint16_t b, uint16_t c, uint16_t d);
		void addQuadAlt(uint16_t a, uint16_t b, uint16_t c, uint16_t d);
		void clear();
		double distance(const DirectX::SimpleMath::Vector3& p, uint16_t tidx) const;
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
		uint16_t v0;
		uint16_t v1;

		Edge(uint16_t v0, uint16_t v1) :
			v0(v0 < v1 ? v0 : v1),
			v1(v0 < v1 ? v1 : v0) {}

		bool operator <(const Edge& rhs) const
		{
			return v0 < rhs.v0 || (v0 == rhs.v0 && v1 < rhs.v1);
		}
	};

	static uint16_t SubdivideEdge(uint16_t f0, uint16_t f1, const DirectX::SimpleMath::Vector3& v0, const DirectX::SimpleMath::Vector3& v1, Mesh& io_mesh, std::map<Edge, uint16_t>& io_divisions);
	static void SubdivideMesh(const Mesh& meshIn, Mesh& meshOut);
	static void Icosahedron(Mesh& mesh);
};



