#pragma once


double dot(const DirectX::SimpleMath::Vector3& a, const DirectX::SimpleMath::Vector3& b);
DirectX::SimpleMath::Vector3 cross(const DirectX::SimpleMath::Vector3& a, const DirectX::SimpleMath::Vector3& b);
double length(const DirectX::SimpleMath::Vector3& a);
DirectX::SimpleMath::Vector3 normalize(const DirectX::SimpleMath::Vector3& a);

struct Mesh
{
	std::vector<DirectX::SimpleMath::Vector3> vertices;
	std::vector<uint32_t> triangles;

	uint32_t triangleCount() const;
	void addTriangle(uint32_t a, uint32_t b, uint32_t c);
	void addQuad(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
	void addQuadAlt(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
	void clear();
	double distance(const DirectX::SimpleMath::Vector3& p, uint32_t tidx) const;
	double distance(const DirectX::SimpleMath::Vector3& p) const;
};

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

void Icosahedron(Mesh& mesh);

void SubdivideMesh(const Mesh& meshIn, Mesh& meshOut);
