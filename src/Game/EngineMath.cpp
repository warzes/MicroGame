#include "stdafx.h"
#include "EngineMath.h"

extern "C" 
{
	int tri_tri_intersect(float V0[3], float V1[3], float V2[3], float U0[3], float U1[3], float U2[3]);
};
//-----------------------------------------------------------------------------
//=============================================================================
// Algebra
//=============================================================================
//-----------------------------------------------------------------------------
void Transform::SetDefault()
{
	m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	m_worldPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	m_worldScale = glm::vec3(1.0f, 1.0f, 1.0f);
	m_rotation = glm::quat(glm::vec3(0.0f));
	m_worldRotation = glm::quat(glm::vec3(0.0f));

	m_forward = glm::vec3(0.0f, 0.0f, 1.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	m_right = glm::vec3(1.0f, 0.0f, 0.0f);

	m_worldMatrix = glm::mat4(1.0f);

	m_isTransformChanged = TransformChanged::TRANSLATION;
}
//-----------------------------------------------------------------------------
void Transform::Translate(const glm::vec3& position)
{
	m_position = position;
	m_isTransformChanged |= TransformChanged::TRANSLATION;
}
//-----------------------------------------------------------------------------
void Transform::Move(const glm::vec3& position)
{
	m_position += position;
	m_isTransformChanged |= TransformChanged::TRANSLATION;
}
//-----------------------------------------------------------------------------
void Transform::RotateEuler(float x, float y, float z)
{
	m_rotation = m_rotation * glm::quat_cast(glm::eulerAngleYXZ(y, x, z));
	m_isTransformChanged |= TransformChanged::ROTATION;
}
//-----------------------------------------------------------------------------
void Transform::Rotate(const glm::quat& rotation)
{
	m_rotation = m_rotation * rotation;
	m_isTransformChanged |= TransformChanged::ROTATION;
}
//-----------------------------------------------------------------------------
void Transform::Scale(const glm::vec3& scale)
{
	m_scale = scale;
	m_isTransformChanged |= TransformChanged::SCALE;
}
//-----------------------------------------------------------------------------
void Transform::updateTransforms()
{
	if (m_isTransformChanged & TransformChanged::NONE) return;

	// Calculate PhysicWorld Matrix
	m_worldMatrix = glm::translate(m_position) * glm::toMat4(m_rotation) * glm::scale(m_scale);

	if (m_parent)
		m_worldMatrix *= m_parent->m_worldMatrix; // TODO: делать дерево наследования, сейчас только одна ветвь

	// Get PhysicWorld Transform
	glm::vec3 pos, scale, skew; glm::vec4 perpective; glm::quat rot;
	if (glm::decompose(m_worldMatrix, scale, rot, pos, skew, perpective))
	{
		m_worldPosition = pos;
		m_worldScale = scale;
		m_worldRotation = rot;
	}

	m_forward = rot * glm::vec3(0.0f, 0.0f, 1.0f);
	m_right = rot * glm::vec3(1.0f, 0.0f, 0.0f);
	m_up = glm::cross(m_forward, m_right);

	m_isTransformChanged = TransformChanged::NONE;
}
//-----------------------------------------------------------------------------
bool Triangle::Intersect(const Triangle& t) const
{
	return (tri_tri_intersect(
		(float*)&verts[0].x,
		(float*)&verts[1].x,
		(float*)&verts[2].x,
		(float*)&t.verts[0].x,
		(float*)&t.verts[1].x,
		(float*)&t.verts[2].x) != 0);
}
//-----------------------------------------------------------------------------
bool Triangle::Intersect(const glm::vec3& O, const glm::vec3& D, glm::vec3& cp, float& tparm, float segmax) const
{
	Plane p(verts[0], verts[1], verts[2]);
	float denom = glm::dot(p.normal, D);

	if (std::fabs(denom) < 1e-8f) return false; // FuzzyIsNull
	float t = -(p.equation.w + glm::dot(p.normal, O)) / denom;
	if (t <= 0.0f) return false;
	if (t > segmax) return false;
	TriangleDesc td(*this, p);
	cp = O + t * D;
	if (td.PointInTri(cp))
	{
		tparm = t;
		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
bool Triangle::Intersect(const glm::vec3& O, float radius, glm::vec3* cp) const
{
	if (cp == nullptr)
		return false;

	glm::vec3& rcp = *cp;
	const Plane p(this->verts[0], this->verts[1], this->verts[2]);
	const float dist = p.SignedDistanceTo(O);
	if (abs(dist) > radius)
		return false;
	glm::vec3 point = O - dist * p.normal;
	TriangleDesc td(*this, p);
	if (td.PointInTri(point)) 
	{
		rcp = point;
		return true;
	}
	/////////////////////////////////////////////////////////
	// Added code for edge intersection detection
	const glm::vec3* v[] = { &this->verts[0], &this->verts[1], &this->verts[2], &this->verts[0] };
	for (int i = 0; i < 3; ++i) 
	{
		const glm::vec3 u(*v[i + 1] - *v[i]), pa(O - *v[i]);
		const float s = glm::dot(u, pa) / LengthSq(u);
		if (s < 0)
			rcp = *v[i];
		else if (s > 1)
			rcp = *v[i + 1];
		else
			rcp = *v[i] + s * u;
		const float sq_dist = LengthSq(O - rcp);
		if (sq_dist < (radius * radius))
			return true;
	}
	/////////////////////////////////////////////////////////
	return false;
}
//-----------------------------------------------------------------------------
void FrustumCorners::Transform(glm::mat4 space)
{
	//move corners of the near plane
	na = glm::vec3(space * glm::vec4(na, 0));
	nb = glm::vec3(space * glm::vec4(nb, 0));
	nc = glm::vec3(space * glm::vec4(nc, 0));
	nd = glm::vec3(space * glm::vec4(nd, 0));
	//move corners of the far plane
	fa = glm::vec3(space * glm::vec4(fa, 0));
	fb = glm::vec3(space * glm::vec4(fb, 0));
	fc = glm::vec3(space * glm::vec4(fc, 0));
	fd = glm::vec3(space * glm::vec4(fd, 0));
}

//create transforms to prevent transforming every triangle into world space
void OldFrustum::SetCullTransform(glm::mat4 objectWorld)
{
	m_cullWorld = objectWorld;
	m_cullInverse = glm::inverse(objectWorld);
}

void OldFrustum::Set(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up, const glm::vec3& right, float nearPlane, float farPlane, float FOV)
{
	m_position = position;
	m_forward = forward;
	m_up = up;
	m_right = right;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
	m_FOV = FOV;
}

void OldFrustum::Update(float aspectRatio)
{
	//calculate generalized relative width and aspect ratio
	float normHalfWidth = tan(glm::radians(m_FOV));

	//calculate width and height for near and far plane
	float nearHW = normHalfWidth * m_nearPlane;
	float nearHH = nearHW / aspectRatio;
	float farHW = normHalfWidth * m_farPlane;// *0.5f;
	float farHH = farHW / aspectRatio;

	//calculate near and far plane centers
	auto nCenter = m_position + m_forward * m_nearPlane;
	auto fCenter = m_position + m_forward * m_farPlane * 0.5f;

	//construct corners of the near plane in the culled objects world space
	m_corners.na = nCenter + m_up * nearHH - m_right * nearHW;
	m_corners.nb = nCenter + m_up * nearHH + m_right * nearHW;
	m_corners.nc = nCenter - m_up * nearHH - m_right * nearHW;
	m_corners.nd = nCenter - m_up * nearHH + m_right * nearHW;
	//construct corners of the far plane
	m_corners.fa = fCenter + m_up * farHH - m_right * farHW;
	m_corners.fb = fCenter + m_up * farHH + m_right * farHW;
	m_corners.fc = fCenter - m_up * farHH - m_right * farHW;
	m_corners.fd = fCenter - m_up * farHH + m_right * farHW;
	m_corners.Transform(m_cullInverse);

	m_positionObject = glm::vec3(m_cullInverse * glm::vec4(m_position, 0));
	m_radInvFOV = 1 / glm::radians(m_FOV);

	//construct planes
	m_planes.clear();
	//winding in an outside perspective so the cross product creates normals pointing inward
	m_planes.push_back(Plane2(m_corners.na, m_corners.nb, m_corners.nc));//Near
	m_planes.push_back(Plane2(m_corners.fb, m_corners.fa, m_corners.fd));//Far 
	m_planes.push_back(Plane2(m_corners.fa, m_corners.na, m_corners.fc));//Left
	m_planes.push_back(Plane2(m_corners.nb, m_corners.fb, m_corners.nd));//Right
	m_planes.push_back(Plane2(m_corners.fa, m_corners.fb, m_corners.na));//Top
	m_planes.push_back(Plane2(m_corners.nc, m_corners.nd, m_corners.fc));//Bottom
}

VolumeCheck OldFrustum::ContainsPoint(const glm::vec3& point) const
{
	for (auto plane : m_planes)
	{
		if (glm::dot(plane.n, point - plane.d) < 0)return VolumeCheck::OUTSIDE;
	}
	return VolumeCheck::CONTAINS;
}
VolumeCheck OldFrustum::ContainsSphere(const Sphere& sphere) const
{
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for (auto plane : m_planes)
	{
		float dist = glm::dot(plane.n, sphere.position - plane.d);
		if (dist < -sphere.radius)return VolumeCheck::OUTSIDE;
		else if (dist < 0) ret = VolumeCheck::INTERSECT;
	}
	return ret;
}
//this method will treat triangles as intersecting even though they may be outside
//but it is faster then performing a proper intersection test with every plane
//and it does not reject triangles that are inside but with all corners outside
VolumeCheck OldFrustum::ContainsTriangle(glm::vec3& a, glm::vec3& b, glm::vec3& c)
{
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for (auto plane : m_planes)
	{
		char rejects = 0;
		if (glm::dot(plane.n, a - plane.d) < 0)rejects++;
		if (glm::dot(plane.n, b - plane.d) < 0)rejects++;
		if (glm::dot(plane.n, c - plane.d) < 0)rejects++;
		// if all three are outside a plane the triangle is outside the frustrum
		if (rejects >= 3)return VolumeCheck::OUTSIDE;
		// if at least one is outside the triangle intersects at least one plane
		else if (rejects > 0)ret = VolumeCheck::INTERSECT;
	}
	return ret;
}
//same as above but with a volume generated above the triangle
VolumeCheck OldFrustum::ContainsTriVolume(glm::vec3& a, glm::vec3& b, glm::vec3& c, float height)
{
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for (auto plane : m_planes)
	{
		char rejects = 0;
		if (glm::dot(plane.n, a - plane.d) < 0)rejects++;
		if (glm::dot(plane.n, b - plane.d) < 0)rejects++;
		if (glm::dot(plane.n, c - plane.d) < 0)rejects++;
		// if all three are outside a plane the triangle is outside the frustrum
		if (rejects >= 3)
		{
			if (glm::dot(plane.n, (a * height) - plane.d) < 0)rejects++;
			if (glm::dot(plane.n, (b * height) - plane.d) < 0)rejects++;
			if (glm::dot(plane.n, (c * height) - plane.d) < 0)rejects++;
			if (rejects >= 6)return VolumeCheck::OUTSIDE;
			else ret = VolumeCheck::INTERSECT;
		}
		// if at least one is outside the triangle intersects at least one plane
		else if (rejects > 0)ret = VolumeCheck::INTERSECT;
	}
	return ret;
}