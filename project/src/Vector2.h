#pragma once

namespace dae
{
	struct Vector2
	{
		float x{};
		float y{};

		Vector2() = default;
		Vector2(float _x, float _y);
		Vector2(const Vector2& from, const Vector2& to);

		float Magnitude() const;
		float SqrMagnitude() const;
		float Normalize();
		Vector2 Normalized() const;

		static float Dot(const Vector2& v1, const Vector2& v2);
		static float Cross(const Vector2& v1, const Vector2& v2);

		//Member Operators
		Vector2 operator*(float scale) const;
		Vector2 operator/(float scale) const;
		Vector2 operator+(const Vector2& v) const;
		Vector2 operator-(const Vector2& v) const;
		Vector2 operator-() const;
		//Vector2& operator-();
		Vector2& operator+=(const Vector2& v);
		Vector2& operator-=(const Vector2& v);
		Vector2& operator/=(float scale);
		Vector2& operator*=(float scale);
		float& operator[](int index);
		float operator[](int index) const;

		static Vector2 Interpolate(const Vector2& v0, const Vector2& v1, const Vector2& v2, const float w0, const float w1, const float w2,
			const float interpolationScale0, const float interpolationScale1, const float interpolationScale2,
			const float interpolatedDepth, const float wProduct)
		{
			return (v0 * w1 * w2 * interpolationScale0 +
				v1 * w0 * w2 * interpolationScale1 +
				v2 * w0 * w1 * interpolationScale2) *
				interpolatedDepth / wProduct;
		}


		static const Vector2 UnitX;
		static const Vector2 UnitY;
		static const Vector2 Zero;
	};

	//Global Operators
	inline Vector2 operator*(float scale, const Vector2& v)
	{
		return { v.x * scale, v.y * scale };
	}
}
