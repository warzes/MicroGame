#include "sysdep.h"
#include "cdmath3d.h"

namespace coldet
{
	const Vector3D Vector3D::Zero(0.0f, 0.0f, 0.0f);
	const Matrix3D Matrix3D::Identity(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	inline float Minor(const Matrix3D& m, const int r0, const int r1, const int r2, const int c0, const int c1, const int c2)
	{
		return m(r0, c0) * (m(r1, c1) * m(r2, c2) - m(r2, c1) * m(r1, c2)) -
			m(r0, c1) * (m(r1, c0) * m(r2, c2) - m(r2, c0) * m(r1, c2)) +
			m(r0, c2) * (m(r1, c0) * m(r2, c1) - m(r2, c0) * m(r1, c1));
	}

	Matrix3D
		Matrix3D::Adjoint() const
	{
		return Matrix3D(
			Minor(*this, 1, 2, 3, 1, 2, 3),
			-Minor(*this, 0, 2, 3, 1, 2, 3),
			Minor(*this, 0, 1, 3, 1, 2, 3),
			-Minor(*this, 0, 1, 2, 1, 2, 3),

			-Minor(*this, 1, 2, 3, 0, 2, 3),
			Minor(*this, 0, 2, 3, 0, 2, 3),
			-Minor(*this, 0, 1, 3, 0, 2, 3),
			Minor(*this, 0, 1, 2, 0, 2, 3),

			Minor(*this, 1, 2, 3, 0, 1, 3),
			-Minor(*this, 0, 2, 3, 0, 1, 3),
			Minor(*this, 0, 1, 3, 0, 1, 3),
			-Minor(*this, 0, 1, 2, 0, 1, 3),

			-Minor(*this, 1, 2, 3, 0, 1, 2),
			Minor(*this, 0, 2, 3, 0, 1, 2),
			-Minor(*this, 0, 1, 3, 0, 1, 2),
			Minor(*this, 0, 1, 2, 0, 1, 2));
	}


	float Matrix3D::Determinant() const
	{
		return 
			m[0][0] * Minor(*this, 1, 2, 3, 1, 2, 3) -
			m[0][1] * Minor(*this, 1, 2, 3, 0, 2, 3) +
			m[0][2] * Minor(*this, 1, 2, 3, 0, 1, 3) -
			m[0][3] * Minor(*this, 1, 2, 3, 0, 1, 2);
	}

	Matrix3D Matrix3D::Inverse() const
	{
		return (1.0f / Determinant()) * Adjoint();
	}

} // namespace coldet