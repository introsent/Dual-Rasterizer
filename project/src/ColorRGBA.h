#pragma once
#include "MathHelpers.h"

namespace dae
{
	struct ColorRGBA
	{
		float r{};
		float g{};
		float b{};
		float a{};


		static ColorRGBA Lerp(const ColorRGBA& c1, const ColorRGBA& c2, float factor)
		{
			return { Lerpf(c1.r, c2.r, factor), Lerpf(c1.g, c2.g, factor), Lerpf(c1.b, c2.b, factor),  Lerpf(c1.a, c2.a, factor) };
		}

		static ColorRGB GetColorRGB(const ColorRGBA& c1)
		{
			return {c1.r, c1.g, c1.b };
		}

#pragma region ColorRGBA (Member) Operators
		const ColorRGBA& operator+=(const ColorRGBA& c)
		{
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;

			return *this;
		}

		ColorRGBA operator+(const ColorRGBA& c) const
		{
			return { r + c.r, g + c.g, b + c.b };
		}

		const ColorRGBA& operator-=(const ColorRGBA& c)
		{
			r -= c.r;
			g -= c.g;
			b -= c.b;
			a -= c.a;

			return *this;
		}

		ColorRGBA operator-(const ColorRGBA& c) const
		{
			return { r - c.r, g - c.g, b - c.b, a - c.a };
		}

		const ColorRGBA& operator*=(const ColorRGBA& c)
		{
			r *= c.r;
			g *= c.g;
			b *= c.b;
			a *= c.a;

			return *this;
		}

		ColorRGBA operator*(const ColorRGBA& c) const
		{
			return { r * c.r, g * c.g, b * c.b, a * c.a };
		}

		const ColorRGBA& operator/=(const ColorRGBA& c)
		{
			r /= c.r;
			g /= c.g;
			b /= c.b;
			a /= c.a;

			return *this;
		}

		const ColorRGBA& operator*=(float s)
		{
			r *= s;
			g *= s;
			b *= s;
			a *= s;

			return *this;
		}

		ColorRGBA operator*(float s) const
		{
			return { r * s, g * s,b * s, a * s};
		}

		const ColorRGBA& operator/=(float s)
		{
			r /= s;
			g /= s;
			b /= s;
			a /= s;

			return *this;
		}

		ColorRGBA operator/(float s) const
		{
			return { r / s, g / s,b / s, a / s};
		}
#pragma endregion
	};

	//ColorRGBA (Global) Operators
	inline ColorRGBA operator*(float s, const ColorRGBA& c)
	{
		return c * s;
	}
}