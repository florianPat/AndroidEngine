#pragma once

#include "Vector2.h"
#include "Utils.h"
#include <math.h>

struct Mat4x4
{
	float matrix[4 * 4] = { 0 };
public:
	Mat4x4() = default;
	//col * 4 + row
	inline Mat4x4& operator*=(const Mat4x4& rhs)
	{
		Mat4x4 result;

		result.matrix[0 * 4 + 0] = matrix[0 * 4 + 0] * rhs.matrix[0 * 4 + 0] + matrix[1 * 4 + 0] * rhs.matrix[0 * 4 + 1] + matrix[2 * 4 + 0] * rhs.matrix[0 * 4 + 2] + matrix[3 * 4 + 0] * rhs.matrix[0 * 4 + 3];
		
		result.matrix[1 * 4 + 0] = matrix[0 * 4 + 0] * rhs.matrix[1 * 4 + 0] + matrix[1 * 4 + 0] * rhs.matrix[1 * 4 + 1] + matrix[2 * 4 + 0] * rhs.matrix[1 * 4 + 2] + matrix[3 * 4 + 0] * rhs.matrix[1 * 4 + 3];
		result.matrix[2 * 4 + 0] = matrix[0 * 4 + 0] * rhs.matrix[2 * 4 + 0] + matrix[1 * 4 + 0] * rhs.matrix[2 * 4 + 1] + matrix[2 * 4 + 0] * rhs.matrix[2 * 4 + 2] + matrix[3 * 4 + 0] * rhs.matrix[2 * 4 + 3];
		result.matrix[3 * 4 + 0] = matrix[0 * 4 + 0] * rhs.matrix[3 * 4 + 0] + matrix[1 * 4 + 0] * rhs.matrix[3 * 4 + 1] + matrix[2 * 4 + 0] * rhs.matrix[3 * 4 + 2] + matrix[3 * 4 + 0] * rhs.matrix[3 * 4 + 3];
		
		result.matrix[0 * 4 + 1] = matrix[0 * 4 + 1] * rhs.matrix[0 * 4 + 0] + matrix[1 * 4 + 1] * rhs.matrix[0 * 4 + 1] + matrix[2 * 4 + 1] * rhs.matrix[0 * 4 + 2] + matrix[3 * 4 + 1] * rhs.matrix[0 * 4 + 3];
		result.matrix[0 * 4 + 2] = matrix[0 * 4 + 2] * rhs.matrix[0 * 4 + 0] + matrix[1 * 4 + 2] * rhs.matrix[0 * 4 + 1] + matrix[2 * 4 + 2] * rhs.matrix[0 * 4 + 2] + matrix[3 * 4 + 2] * rhs.matrix[0 * 4 + 3];
		result.matrix[0 * 4 + 3] = matrix[0 * 4 + 3] * rhs.matrix[0 * 4 + 0] + matrix[1 * 4 + 3] * rhs.matrix[0 * 4 + 1] + matrix[2 * 4 + 3] * rhs.matrix[0 * 4 + 2] + matrix[3 * 4 + 3] * rhs.matrix[0 * 4 + 3];
		
		result.matrix[1 * 4 + 1] = matrix[0 * 4 + 1] * rhs.matrix[1 * 4 + 0] + matrix[1 * 4 + 1] * rhs.matrix[1 * 4 + 1] + matrix[2 * 4 + 1] * rhs.matrix[1 * 4 + 2] + matrix[3 * 4 + 1] * rhs.matrix[1 * 4 + 3];
		result.matrix[1 * 4 + 2] = matrix[0 * 4 + 2] * rhs.matrix[1 * 4 + 0] + matrix[1 * 4 + 2] * rhs.matrix[1 * 4 + 1] + matrix[2 * 4 + 2] * rhs.matrix[1 * 4 + 2] + matrix[3 * 4 + 2] * rhs.matrix[1 * 4 + 3];
		result.matrix[1 * 4 + 3] = matrix[0 * 4 + 3] * rhs.matrix[1 * 4 + 0] + matrix[1 * 4 + 3] * rhs.matrix[1 * 4 + 1] + matrix[2 * 4 + 3] * rhs.matrix[1 * 4 + 2] + matrix[3 * 4 + 3] * rhs.matrix[1 * 4 + 3];
		
		result.matrix[2 * 4 + 1] = matrix[0 * 4 + 1] * rhs.matrix[2 * 4 + 0] + matrix[1 * 4 + 1] * rhs.matrix[2 * 4 + 1] + matrix[2 * 4 + 1] * rhs.matrix[2 * 4 + 2] + matrix[3 * 4 + 1] * rhs.matrix[2 * 4 + 3];
		result.matrix[2 * 4 + 2] = matrix[0 * 4 + 2] * rhs.matrix[2 * 4 + 0] + matrix[1 * 4 + 2] * rhs.matrix[2 * 4 + 1] + matrix[2 * 4 + 2] * rhs.matrix[2 * 4 + 2] + matrix[3 * 4 + 2] * rhs.matrix[2 * 4 + 3];
		result.matrix[2 * 4 + 3] = matrix[0 * 4 + 3] * rhs.matrix[2 * 4 + 0] + matrix[1 * 4 + 3] * rhs.matrix[2 * 4 + 1] + matrix[2 * 4 + 3] * rhs.matrix[2 * 4 + 2] + matrix[3 * 4 + 3] * rhs.matrix[2 * 4 + 3];
		
		result.matrix[3 * 4 + 1] = matrix[0 * 4 + 1] * rhs.matrix[3 * 4 + 0] + matrix[1 * 4 + 1] * rhs.matrix[3 * 4 + 1] + matrix[2 * 4 + 1] * rhs.matrix[3 * 4 + 2] + matrix[3 * 4 + 1] * rhs.matrix[3 * 4 + 3];
		result.matrix[3 * 4 + 2] = matrix[0 * 4 + 2] * rhs.matrix[3 * 4 + 0] + matrix[1 * 4 + 2] * rhs.matrix[3 * 4 + 1] + matrix[2 * 4 + 2] * rhs.matrix[3 * 4 + 2] + matrix[3 * 4 + 2] * rhs.matrix[3 * 4 + 3];
		result.matrix[3 * 4 + 3] = matrix[0 * 4 + 3] * rhs.matrix[3 * 4 + 0] + matrix[1 * 4 + 3] * rhs.matrix[3 * 4 + 1] + matrix[2 * 4 + 3] * rhs.matrix[3 * 4 + 2] + matrix[3 * 4 + 3] * rhs.matrix[3 * 4 + 3];

		*this = result;

		return *this;
	}

	inline Mat4x4 operator*(const Mat4x4& rhs) const
	{
		Mat4x4 lhs = *this;
		lhs *= rhs;
		return lhs;
	}

	inline Vector2f operator*(const Vector2f& rhs) const
	{
		Vector2f result;

		result.x = matrix[0 * 4 + 0] * rhs.x + matrix[1 * 4 + 0] * rhs.y + matrix[3 * 4 + 0] * 1.0f;
		result.y = matrix[0 * 4 + 1] * rhs.x + matrix[1 * 4 + 1] * rhs.y + matrix[3 * 4 + 1] * 1.0f;

		return result;
	}

	static Mat4x4 identity()
	{
		Mat4x4 result;

		result.matrix[0 * 4 + 0] = 1;
		result.matrix[1 * 4 + 1] = 1;
		result.matrix[2 * 4 + 2] = 1;
		result.matrix[3 * 4 + 3] = 1;

		return result;
	}

	static Mat4x4 translate(const Vector2f& trans)
	{
		Mat4x4 result = identity();

		result.matrix[3 * 4 + 0] = trans.x;
		result.matrix[3 * 4 + 1] = trans.y;

		return result;
	}

	static Mat4x4 scale(const Vector2f& scl)
	{
		Mat4x4 result = identity();

		result.matrix[0 * 4 + 0] = scl.x;
		result.matrix[1 * 4 + 1] = scl.y;

		return result;
	}

	static Mat4x4 rotate(const float rot)
	{
		Mat4x4 result = identity();

		if (rot != 0.0f)
		{
			float cosA = cosf(utils::degreesToRadians(rot));
			float sinA = sinf(utils::degreesToRadians(rot));

			Mat4x4 rot = identity();
			rot.matrix[0 * 4 + 0] = cosA;
			rot.matrix[1 * 4 + 0] = sinA;
			rot.matrix[0 * 4 + 1] = -sinA;
			rot.matrix[1 * 4 + 1] = cosA;

			result *= rot;
		}

		return result;
	}

	static Mat4x4 orthoProj(float nearPlane, float farPlane, float left, float bottom, float right, float top)
	{
		Mat4x4 result = identity();

		result.matrix[0 * 4 + 0] = 2 / (right - left);
		result.matrix[1 * 4 + 1] = 2 / (top - bottom);
		result.matrix[2 * 4 + 2] = -(2 / (farPlane - nearPlane));
		result.matrix[3 * 4 + 0] = -(right + left) / (right - left);
		result.matrix[3 * 4 + 1] = -(top + bottom) / (top - bottom);
		result.matrix[3 * 4 + 2] = -((farPlane + nearPlane) / (farPlane - nearPlane));

		return result;
	}
};