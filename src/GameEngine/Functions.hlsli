#pragma once

double _pow(double a, int b)
{
	double rv = 1.;
	for (int i = 0; i < b; i++) { rv *= a; }
	return rv;
}

double3 _pow(double3 x, int y)
{
	double3 rv;
	for (int i = 0; i < 3; i++) { rv[i] = _pow(x[i], y); }
	return rv;
}

double _rsqrt(double a) { return (double)pow((float)a, -.5); }
double3 _rsqrt(double3 a) { return (double3)pow((float3)a, -.5); }

double _sqrt(double a) { return 1. / _rsqrt(a); }
double3 _sqrt(double3 a) { return 1. / _rsqrt(a); }

double _min(double3 x)
{
	if (x.x < x.y && x.x < x.z)
		return x.x;
	if (x.z < x.x && x.z < x.y)
		return x.z;
	return x.y;
}

double _max(double3 x)
{
	if (x.x > x.y && x.x > x.z)
		return x.x;
	if (x.z > x.x && x.z > x.y)
		return x.z;
	return x.y;
}

double _dot(double3 a, double3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
double _dot(double4 a, double4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

double3 _normalize(double3 a) { return a / _sqrt(_dot(a, a)); }

double3 _cross(double3 a, double3 b) { return a.yzx * b.zxy - a.zxy * b.yzx; }

double3 _mul(double3 v, double3x3 M)
{
	double3 r;
	r.x = _dot(M._m00_m01_m02, v);
	r.y = _dot(M._m10_m11_m12, v);
	r.z = _dot(M._m20_m21_m22, v);
	return r;
}

double _determinant(double3x3 A)
{
	return _dot(A._m00_m01_m02,
	            A._m11_m12_m10 * A._m22_m20_m21
	            - A._m12_m10_m11 * A._m21_m22_m20);
}

double3x3 _inverse(double3x3 A)
{
	double3x3 C;

	double det = _determinant(A);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			C[i][j] = (A[(j + 1) % 3][(i + 1) % 3] * A[(j + 2) % 3][(i + 2) % 3]) - (A[(j + 1) % 3][(i + 2) % 3] * A[(j
				+ 2) % 3][(i + 1) % 3]);

	return C / det;
}

float _scale(float value, float limit)
{
	return min(max(value, -limit), limit);
}

// https://gist.github.com/kevinmoran/b45980723e53edeb8a5a43c49f134724
float3x3 _rotateAlign(float3 u1, float3 u2)
{
	float3 axis = cross(u1, u2);

	const float cosA = dot(u1, u2);
	const float k = 1.0f / (1.0f + cosA);

	float3x3 result = {
		(axis.x * axis.x * k) + cosA, (axis.y * axis.x * k) - axis.z, (axis.z * axis.x * k) + axis.y,
		(axis.x * axis.y * k) + axis.z, (axis.y * axis.y * k) + cosA, (axis.z * axis.y * k) - axis.x,
		(axis.x * axis.z * k) - axis.y, (axis.y * axis.z * k) + axis.x, (axis.z * axis.z * k) + cosA
	};

	return result;
}

float3 _rotate(float3 x, float3 dir)
{
	float cosX = cos(dir.x);
	float cosY = cos(dir.y);
	float cosZ = cos(dir.z);
	float sinX = sin(dir.x);
	float sinY = sin(dir.y);
	float sinZ = sin(dir.z);

	float3x3 rotation = {
		cosX * cosY * cosZ - sinX * sinZ, -cosX * cosY * sinZ - sinX * cosZ, cosX * sinY,
		sinX * cosY * cosZ + cosX * sinZ, -sinX * cosY * sinZ + cosX * cosZ, sinX * sinY,
		-sinY * cosZ, sinY * sinZ, cosY
	};

	return mul(x, rotation).xyz;
}

double _distance(double3 x, double3 y) { return _sqrt(_pow(y.x - x.x, 2) + _pow(y.y - x.y, 2) + _pow(y.z - x.z, 2)); }

double toScreen(double v) { return v * S_NORM_INV; }
double2 toScreen(double2 v) { return v * S_NORM_INV; }
double3 toScreen(double3 v) { return v * S_NORM_INV; }
double4 toScreen(double4 v) { return v * S_NORM_INV; }

double toReal(double v) { return v * S_NORM; }
double2 toReal(double2 v) { return v * S_NORM; }
double3 toReal(double3 v) { return v * S_NORM; }
double4 toReal(double4 v) { return v * S_NORM; }
