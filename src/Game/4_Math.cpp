#include "stdafx.h"
#include "4_Math.h"

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
void Frustum::SetCullTransform(glm::mat4 objectWorld)
{
	m_cullWorld = objectWorld;
	m_cullInverse = glm::inverse(objectWorld);
}

void Frustum::Set(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up, const glm::vec3& right, float nearPlane, float farPlane, float FOV)
{
	m_position = position;
	m_forward = forward;
	m_up = up;
	m_right = right;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
	m_FOV = FOV;
}

void Frustum::Update(float aspectRatio)
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
	m_planes.push_back(Plane(m_corners.na, m_corners.nb, m_corners.nc));//Near
	m_planes.push_back(Plane(m_corners.fb, m_corners.fa, m_corners.fd));//Far 
	m_planes.push_back(Plane(m_corners.fa, m_corners.na, m_corners.fc));//Left
	m_planes.push_back(Plane(m_corners.nb, m_corners.fb, m_corners.nd));//Right
	m_planes.push_back(Plane(m_corners.fa, m_corners.fb, m_corners.na));//Top
	m_planes.push_back(Plane(m_corners.nc, m_corners.nd, m_corners.fc));//Bottom
}

VolumeCheck Frustum::ContainsPoint(const glm::vec3& point) const
{
	for (auto plane : m_planes)
	{
		if (glm::dot(plane.n, point - plane.d) < 0)return VolumeCheck::OUTSIDE;
	}
	return VolumeCheck::CONTAINS;
}
VolumeCheck Frustum::ContainsSphere(const Sphere& sphere) const
{
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for (auto plane : m_planes)
	{
		float dist = glm::dot(plane.n, sphere.pos - plane.d);
		if (dist < -sphere.radius)return VolumeCheck::OUTSIDE;
		else if (dist < 0) ret = VolumeCheck::INTERSECT;
	}
	return ret;
}
//this method will treat triangles as intersecting even though they may be outside
//but it is faster then performing a proper intersection test with every plane
//and it does not reject triangles that are inside but with all corners outside
VolumeCheck Frustum::ContainsTriangle(glm::vec3& a, glm::vec3& b, glm::vec3& c)
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
VolumeCheck Frustum::ContainsTriVolume(glm::vec3& a, glm::vec3& b, glm::vec3& c, float height)
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