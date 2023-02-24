#pragma once
class Vec
{
public:
	float x, y, z, w;

	Vec() {
		x = 0.0;
		y = 0.0;
		z = 0.0;
		w = 0.0;
	}

	Vec(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	Vec operator+(Vec o) {
		return Vec(x + o.x, y + o.y, z + o.z, w + o.w);
	}

	Vec operator-(Vec o) {
		return Vec(x - o.x, y - o.y, z - o.z, w - o.w);
	}

	Vec operator*(float s) {
		return Vec(s * x, s * y, s * z, s * w);
	}

	Vec operator/(double s) {
		return Vec(x / s, y / s, z / s, w / s);
	}
};
