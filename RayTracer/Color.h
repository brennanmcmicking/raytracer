#pragma once
class Color
{
public:
	double r, g, b, a;

	Color() {
		this->r = 0.0;
		this->g = 0.0;
		this->b = 0.0;
		this->a = 0.0;
	}

	Color(float r, float g, float b, float a) {
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	Color operator+ (const Color &other) {
		return Color(r + other.r, g + other.g, b + other.b, a + other.a);
	}

	Color operator* (const double s) {
		return Color(r * s, g * s, b * s, a * s);
	}

	Color operator/ (const int divider) {
		return Color(r / divider, g / divider, b / divider, 1.0);
	}

	bool operator> (const float o) {
		return (r + g + b) / 3 > o;
	}

	Color operator* (const Color& other) {
		return Color(r * other.r, g * other.g, b * other.b, a * other.a);
	}
};

