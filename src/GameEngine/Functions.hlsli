#pragma once

double _pow(double a, int b) {
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
	else if (x.z < x.x && x.z < x.y)
		return x.z;
	else return x.y;
}

double _max(double3 x)
{
	if (x.x > x.y && x.x > x.z)
		return x.x;
	else if (x.z > x.x && x.z > x.y)
		return x.z;
	else return x.y;
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
			C[i][j] = (A[(j + 1) % 3][(i + 1) % 3] * A[(j + 2) % 3][(i + 2) % 3]) - (A[(j + 1) % 3][(i + 2) % 3] * A[(j + 2) % 3][(i + 1) % 3]);

	return C / det;
}

float3 _rotate(float3 x, float3 dir)
{
	float3x3 rotation = {
		cos(dir.x) * cos(dir.y) * cos(dir.z) - sin(dir.x) * sin(dir.z), -cos(dir.x) * cos(dir.y) * sin(dir.z) - sin(dir.x) * cos(dir.z), cos(dir.x) * sin(dir.y),
		sin(dir.x) * cos(dir.y) * cos(dir.z) + cos(dir.x) * sin(dir.z), -sin(dir.x) * cos(dir.y) * sin(dir.z) + cos(dir.x) * cos(dir.z), sin(dir.x) * sin(dir.y),
		-sin(dir.y) * cos(dir.z), sin(dir.y) * sin(dir.z), cos(dir.y)
	};

	return mul(x, rotation).xyz;

	/*float a = 1;

	float3x3 rotation = {
		cos(a) + pow(up.x, 2) * (1 - cos(a)), up.x * up.y * (1 - cos(a)) - up.z * sin(a), up.x * up.z * (1 - cos(a)) + up.y * sin(a),
		up.y * up.x * (1 - cos(a)) + up.z * sin(a), cos(a) + pow(up.y, 2) * (1 - cos(a)), up.y * up.z * (1 - cos(a)) - up.x * sin(a),
		up.z * up.x * (1 - cos(a)) - up.y * sin(a), up.z * up.y * (1 - cos(a)) + up.x * sin(a), cos(a) + pow(up.z, 2) * (1 - cos(a))
	};*/
}

double _distance(double3 x, double3 y) { return _sqrt(_pow(y.x - x.x, 2) + _pow(y.y - x.y, 2) + _pow(y.z - x.z, 2)); }