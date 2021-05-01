#pragma once

#include "pch.h"
#include "Sphere.h"

#include <cmath>
#include <vector>
#include <array>
#include <map>
#include <random>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace DirectX;
using namespace SimpleMath;

using Microsoft::WRL::ComPtr;

double Sphere::dot(const Vector3& a, const Vector3& b)
{
	return static_cast<double>(a.x) * static_cast<double>(b.x) + static_cast<double>(a.y) * static_cast<double>(b.y) +
		static_cast<double>(a.z) * static_cast<double>(b.z);
}

Vector3 Sphere::cross(const Vector3& a, const Vector3& b)
{
	return Vector3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

double Sphere::length(const Vector3& a)
{
	return std::sqrt(dot(a, a));
}

Vector3 Sphere::normalize(const Vector3& a)
{
	const double lrcp = 1.0 / std::sqrt(dot(a, a));
	return Vector3(a.x * lrcp, a.y * lrcp, a.z * lrcp);
}

uint32_t Sphere::Mesh::triangle_count() const { return indices.size() / 3; }

void Sphere::Mesh::add_triangle(uint32_t a, uint32_t b, uint32_t c)
{
	indices.emplace_back(a);
	indices.emplace_back(b);
	indices.emplace_back(c);
}

void Sphere::Mesh::add_quad(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
	indices.emplace_back(a);
	indices.emplace_back(b);
	indices.emplace_back(c);
	indices.emplace_back(a);
	indices.emplace_back(c);
	indices.emplace_back(d);
}

void Sphere::Mesh::add_quad_alt(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
	indices.emplace_back(a);
	indices.emplace_back(b);
	indices.emplace_back(d);
	indices.emplace_back(b);
	indices.emplace_back(c);
	indices.emplace_back(d);
}

void Sphere::Mesh::clear()
{
	vertices.clear();
	indices.clear();
}

double Sphere::Mesh::distance(const Vector3& p, uint32_t tidx) const
{
	const uint32_t idx0 = indices[tidx];
	const uint32_t idx1 = indices[tidx + 1];
	const uint32_t idx2 = indices[tidx + 2];
	const Vector3 v0 = vertices[idx0];
	const Vector3 v1 = vertices[idx1];
	const Vector3 v2 = vertices[idx2];
	const Vector3 bv = v0;
	const Vector3 e0 = v1 - v0;
	const Vector3 e1 = v2 - v0;
	const Vector3 dv = bv - p;
	const double a = dot(e0, e0);
	const double b = dot(e0, e1);
	const double c = dot(e1, e1);
	const double d = dot(e0, dv);
	const double e = dot(e1, dv);
	const double f = dot(dv, dv);

	const double det = a * c - b * b;
	double s = b * e - c * d;
	double t = b * d - a * e;

	if (s + t <= det)
	{
		if (s < 0.0)
		{
			if (t < 0.0)
			{
				// region 4
				if (d < 0.0)
				{
					t = 0.0;
					s = -d >= a ? 1.0 : -d / a;
				}
				else
				{
					s = 0.0;
					t = e >= 0.0 ? 0.0 : (-e >= c ? 1.0 : -e / c);
				}
			}
			else
			{
				// region 3
				s = 0.0;
				t = e >= 0.0 ? 0.0 : (-e >= c ? 1.0 : -e / c);
			}
		}
		else if (t < 0.0)
		{
			// region 5
			s = d >= 0.0 ? 0.0 : (-d >= a ? 1.0 : -d / a);
			t = 0.0;
		}
		else
		{
			// region 0
			const double invDet = 1.0 / det;
			s *= invDet;
			t *= invDet;
		}
	}
	else
	{
		if (s < 0.0)
		{
			// region 2
			const double tmp0 = b + d;
			const double tmp1 = c + e;
			if (tmp1 > tmp0)
			{
				const double numer = tmp1 - tmp0;
				const double denom = a - 2.0 * b + c;
				s = numer >= denom ? 1.0 : numer / denom;
				t = 1.0 - s;
			}
			else
			{
				s = 0.0;
				t = (tmp1 <= 0.0 ? 1.0 : (e >= 0.0 ? 0.0 : -e / c));
			}
		}
		else if (t < 0.0)
		{
			// region 6
			const double tmp0 = b + e;
			const double tmp1 = a + d;
			if (tmp1 > tmp0)
			{
				const double numer = tmp1 - tmp0;
				const double denom = a - 2.0 * b + c;
				t = numer >= denom ? 1.0 : numer / denom;
				s = 1.0 - t;
			}
			else
			{
				s = (tmp1 <= 0.0 ? 1.0 : (d >= 0.0 ? 0.0 : -d / a));
				t = 0.0;
			}
		}
		else
		{
			// region 1
			const double numer = c + e - b - d;
			if (numer <= 0)
			{
				s = 0.0;
			}
			else
			{
				const double denom = a - 2.0 * b + c;
				s = numer >= denom ? 1.0 : numer / denom;
			}
			t = 1.0 - s;
		}
	}

	return length(p - (v0 + Vector3(s) * e0 + Vector3(t) * e1));
}

double Sphere::Mesh::distance(const Vector3& p) const
{
	double min = 10e10;
	for (uint32_t i = 0; i < indices.size(); i += 3)
	{
		min = std::fmin(min, distance(p, i));
	}
	return min;
}

Sphere::Mesh Sphere::icosahedron()
{
	Mesh mesh{};

	const double t = (1.0 + std::sqrt(5.0)) / 2.0;

	// Vertices
	mesh.vertices.emplace_back(normalize(Vector3(-1.0, t, 0.0)));
	mesh.vertices.emplace_back(normalize(Vector3(1.0, t, 0.0)));
	mesh.vertices.emplace_back(normalize(Vector3(-1.0, -t, 0.0)));
	mesh.vertices.emplace_back(normalize(Vector3(1.0, -t, 0.0)));
	mesh.vertices.emplace_back(normalize(Vector3(0.0, -1.0, t)));
	mesh.vertices.emplace_back(normalize(Vector3(0.0, 1.0, t)));
	mesh.vertices.emplace_back(normalize(Vector3(0.0, -1.0, -t)));
	mesh.vertices.emplace_back(normalize(Vector3(0.0, 1.0, -t)));
	mesh.vertices.emplace_back(normalize(Vector3(t, 0.0, -1.0)));
	mesh.vertices.emplace_back(normalize(Vector3(t, 0.0, 1.0)));
	mesh.vertices.emplace_back(normalize(Vector3(-t, 0.0, -1.0)));
	mesh.vertices.emplace_back(normalize(Vector3(-t, 0.0, 1.0)));

	// Faces
	mesh.add_triangle(0, 11, 5);
	mesh.add_triangle(0, 5, 1);
	mesh.add_triangle(0, 1, 7);
	mesh.add_triangle(0, 7, 10);
	mesh.add_triangle(0, 10, 11);
	mesh.add_triangle(1, 5, 9);
	mesh.add_triangle(5, 11, 4);
	mesh.add_triangle(11, 10, 2);
	mesh.add_triangle(10, 7, 6);
	mesh.add_triangle(7, 1, 8);
	mesh.add_triangle(3, 9, 4);
	mesh.add_triangle(3, 4, 2);
	mesh.add_triangle(3, 2, 6);
	mesh.add_triangle(3, 6, 8);
	mesh.add_triangle(3, 8, 9);
	mesh.add_triangle(4, 9, 5);
	mesh.add_triangle(2, 4, 11);
	mesh.add_triangle(6, 2, 10);
	mesh.add_triangle(8, 6, 7);
	mesh.add_triangle(9, 8, 1);

	return mesh;
}

uint32_t Sphere::subdivide_edge(uint32_t f0, uint32_t f1, const Vector3& v0, const Vector3& v1, Mesh& io_mesh,
                                std::map<edge, uint32_t>& io_divisions)
{
	const edge edge(f0, f1);
	auto it = io_divisions.find(edge);
	if (it != io_divisions.end())
		return it->second;

	const Vector3 v = normalize(Vector3(0.5) * (v0 + v1));
	const uint32_t f = io_mesh.vertices.size();
	io_mesh.vertices.emplace_back(v);
	io_divisions.emplace(edge, f);
	return f;
}

void Sphere::subdivide_mesh(Mesh& mesh)
{
	std::map<edge, uint32_t> divisions; // Edge -> new vertex

	const uint32_t count = mesh.triangle_count();

	std::vector<std::array<uint32_t, 6>> subdivides;

	for (uint32_t i = 0; i < count; i++)
	{
		std::array<uint32_t, 6> f = {
			mesh.indices[i * 3],
			mesh.indices[i * 3 + 1],
			mesh.indices[i * 3 + 2]
		};

		//mesh.indices.erase(mesh.indices.begin() + (i * 3), mesh.indices.begin() + (i * 3 + 3));

		const Vector3 v0 = mesh.vertices[f[0]];
		const Vector3 v1 = mesh.vertices[f[1]];
		const Vector3 v2 = mesh.vertices[f[2]];

		f[3] = subdivide_edge(f[0], f[1], v0, v1, mesh, divisions);
		f[4] = subdivide_edge(f[1], f[2], v1, v2, mesh, divisions);
		f[5] = subdivide_edge(f[2], f[0], v2, v0, mesh, divisions);

		auto info = (std::to_string(f[0]) + "," + std::to_string(f[1]) + "," + std::to_string(f[2]) + "," +
			std::to_string(f[3]) + "," + std::to_string(f[4]) + "," + std::to_string(f[5]));
		subdivides.push_back(f);
	}

	mesh.indices.clear();
	mesh.indices.shrink_to_fit();

	for (std::array<uint32_t, 6> f : subdivides)
	{
		auto info = (std::to_string(f[0]) + "," + std::to_string(f[1]) + "," + std::to_string(f[2]) + "," +
			std::to_string(f[3]) + "," + std::to_string(f[4]) + "," + std::to_string(f[5]));

		mesh.add_triangle(f[0], f[3], f[5]);
		mesh.add_triangle(f[3], f[1], f[4]);
		mesh.add_triangle(f[4], f[2], f[5]);
		mesh.add_triangle(f[3], f[4], f[5]);
	}

	subdivides.clear();
	subdivides.shrink_to_fit();

	mesh.indices_p.clear();
	for (uint32_t i = 0; i < mesh.vertices.size(); i++)
		mesh.indices_p.emplace_back(i);
	mesh.indices_p.shrink_to_fit();
}
