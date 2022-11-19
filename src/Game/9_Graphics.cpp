#include "stdafx.h"
#include "3_Core.h"
#include "4_Math.h"
#include "6_Platform.h"
#include "8_Renderer.h"
#include "9_Graphics.h"

#include <stb/stb_truetype.h>
#include <tiny_obj_loader.h>

static Camera* last_camera = nullptr;

Camera::Camera()
{
	Camera* old = last_camera;

	m_speed = 1;
	m_position = glm::vec3(0, 0, 0);
	m_last_look = m_last_move = glm::vec3(0, 0, 0);
	m_up = glm::vec3(0, 1, 0);

	// @todo: remove this hack
	static bool smoothing = true;// if (smoothing < 0) smoothing = flag("--with-camera-smooth");
	if (smoothing) 
	{
		for (int i = 0; i < 1000; ++i)
		{
			Move(0, 0, 0);
			Fps(0, 0);
		}
		smoothing = false;
	}

	// update proj & view
	LookAt(glm::vec3(0, 0, 1));

	last_camera = old;
}

Camera* Camera::GetActive()
{
	static Camera defaults;
	if (!last_camera)
	{
		defaults.m_view = glm::mat4(1.0f);
		defaults.m_proj = glm::mat4(1.0f);
		last_camera = &defaults;
	}
	return last_camera;
}

void Camera::Teleport(float px, float py, float pz)
{
	m_position = glm::vec3(px, py, pz);
	Fps(0, 0);
}

void Camera::Move(float incx, float incy, float incz)
{
	// enable camera smoothing
	static bool smoothing = true; //if (smoothing < 0) smoothing = flag("--with-camera-smooth");
	if (smoothing) 
	{
		float move_friction = 0.99f;
		m_last_move = tempMath::scale3(m_last_move, move_friction);
		float move_filtering = 0.975f;
		incx = m_last_move.x = incx * (1 - move_filtering) + m_last_move.x * move_filtering;
		incy = m_last_move.y = incy * (1 - move_filtering) + m_last_move.y * move_filtering;
		incz = m_last_move.z = incz * (1 - move_filtering) + m_last_move.z * move_filtering;
		smoothing = false;
	}

	glm::vec3 dir = tempMath::norm3(tempMath::cross3(m_look, m_up));
	m_position = tempMath::add3(m_position, tempMath::scale3(dir, incx)); // right
	m_position = tempMath::add3(m_position, tempMath::scale3(m_up, incy)); // up
	m_position = tempMath::add3(m_position, tempMath::scale3(m_look, incz)); // front

	Fps(0, 0);
}

void Camera::Fps(float yaw, float pitch)
{
	last_camera = this;

	// enable camera smoothing
	static bool smoothing = true; //if (smoothing < 0) smoothing = flag("--with-camera-smooth");
	if (smoothing) 
	{
		float look_friction = 0.999f;
		m_last_look.x *= look_friction;
		m_last_look.y *= look_friction;
		float look_filtering = 0.05f;
		yaw = m_last_look.y = yaw * look_filtering + m_last_look.y * (1 - look_filtering);
		pitch = m_last_look.x = pitch * look_filtering + m_last_look.x * (1 - look_filtering);
		smoothing = false;
	}

	m_yaw += yaw;
	m_yaw = fmod(m_yaw, 360);
	m_pitch += pitch;
	m_pitch = m_pitch > 89 ? 89 : m_pitch < -89 ? -89 : m_pitch;

	const float deg2rad = 0.0174532f, y = m_yaw * deg2rad, p = m_pitch * deg2rad;
	m_look = tempMath::norm3(glm::vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));

	m_view = glm::lookAt(m_position, tempMath::add3(m_position, m_look), m_up);

	//tempMath::lookat44(m_view, m_position, tempMath::add3(m_position, m_look), m_up); // eye,center,up
	tempMath::perspective44(m_proj, 45, GetFrameBufferWidth() / ((float)GetFrameBufferHeight() + !GetFrameBufferHeight()), 0.01f, 1000.f);

#if 0 // isometric/dimetric
#define orthogonal(proj, fov, aspect, znear, zfar) \
    ortho44((proj), -(fov) * (aspect), (fov) * (aspect), -(fov), (fov), (znear), (zfar))

	float DIMETRIC = 30.000f;
	float ISOMETRIC = 35.264f;
	float aspect = window_width() / ((float)window_height() + !!window_height());
	orthogonal(cam->proj, 45, aspect, -1000, 1000); // why -1000?
	// cam->yaw = 45;
	cam->pitch = -ISOMETRIC;
#endif
}

#define concat(a,b)      conc4t(a,b)
#define conc4t(a,b)      a##b

#define macro(name)      concat(name, __LINE__)
//#define defer(begin,end) for(int macro(i) = ((begin), 0); !macro(i); macro(i) = ((end), 1))
//#define scope(end)       defer((void)0, end)
//#define benchmark        for(double macro(t) = -time_ss(); macro(t) < 0; printf("%.2fs (" FILELINE ")\n", macro(t)+=time_ss()))
#define do_once          static int macro(once) = 0; for(;!macro(once);macro(once)=1)


void Camera::Orbit(float yaw, float pitch, float inc_distance)
{
	last_camera = this;

	glm::vec2 inc_mouse = glm::vec2(yaw, pitch);

	// @todo: worth moving all these members into camera_t ?
	static glm::vec2 _mouse = { 0,0 };
	static glm::vec2 _polarity = { +1,-1 };
	static glm::vec2 _sensitivity = { 2,2 };
	static float _friction = 0.75; //99;
	static float _distance; do_once _distance = tempMath::len3(m_position);

	// update dummy state
	Fps(0, 0);

	// add smooth input
	_mouse = tempMath::mix2(_mouse, tempMath::add2(_mouse, tempMath::mul2(tempMath::mul2(inc_mouse, _sensitivity), _polarity)), _friction);
	_distance = tempMath::mixf(_distance, _distance + inc_distance, _friction);

	// look: update angles
	glm::vec2 offset = tempMath::sub2(_mouse, tempMath::ptr2(&m_last_move.x));
	if (1)
	{ // if _enabled
		yaw += offset.x;
		pitch += offset.y;
		// look: limit pitch angle [-89..89]
		pitch = pitch > 89 ? 89 : pitch < -89 ? -89 : pitch;
	}

	// compute view matrix
	float x = tempMath::rad(yaw), y = tempMath::rad(pitch), cx = cosf(x), cy = cosf(y), sx = sinf(x), sy = sinf(y);
	tempMath::lookat44(m_view, glm::vec3(cx * cy * _distance, sy * _distance, sx * cy * _distance), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	// save for next call
	m_last_move.x = _mouse.x;
	m_last_move.y = _mouse.y;
}

void Camera::LookAt(const glm::vec3& target)
{
	// invert expression that cam->look = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
	// look.y = sin p > y = asin(p)
	// look.x = cos y * cos p; -> cos p = look.x / cos y \ look.x / cos y = look.z / sin y
	// look.z = sin y * cos p; -> cos p = look.z / sin y /
	// so, sin y / cos y = look x / look z > tan y = look x / look z > y = atan(look x / look z)

	glm::vec3 look = tempMath::norm3(tempMath::sub3(target, m_position));
	const float rad2deg = 1 / 0.0174532f;
	float npitch = asin(look.y) * rad2deg;
	float nyaw = atan2(look.z, look.x) * rad2deg; // coords swapped. it was (look.x, look.z) before. @todo: testme

	Fps(nyaw - m_yaw, npitch - m_pitch);
}

void Camera::Enable()
{
	// camera_t *other = camera_get_active(); // init default camera in case there is none
	last_camera = this;
	// trigger a dummy update -> update matrices
	Fps(0, 0);
}


namespace DebugDraw
{
	std::map<unsigned, std::vector<glm::vec3>> Points;
	std::map<unsigned, std::vector<glm::vec3>> Lines;

	void drawGround_(float scale)
	{ // 10x10
		// outer
		for (float i = -scale, c = 0; c <= 20; c += 20, i += c * (scale / 10))
		{
			DrawLine(glm::vec3(-scale, 0, i), glm::vec3(+scale, 0, i), WHITE); // horiz
			DrawLine(glm::vec3(i, 0, -scale), glm::vec3(i, 0, +scale), WHITE); // vert
		}
		// inner		
		for (float i = -scale + scale / 10, c = 1; c < 20; ++c, i += (scale / 10))
		{
			DrawLine(glm::vec3(-scale, 0, i), glm::vec3(+scale, 0, i), GRAY); // horiz
			DrawLine(glm::vec3(i, 0, -scale), glm::vec3(i, 0, +scale), GRAY); // vert
		}
	}

	void DrawConeLowres(const glm::vec3& center, const glm::vec3& top, float radius, unsigned rgb)
	{
		glm::vec3 diff3 = tempMath::sub3(top, center);
		DrawPrism(center, radius ? radius : 1, tempMath::len3(diff3), tempMath::norm3(diff3), 3, rgb);
	}

	void DrawCircleWithOrientation(const glm::vec3& center, glm::vec3 dir, float radius, unsigned rgb)
	{
		// we'll skip 3 segments out of 32. 1.5 per half circle.
		int segments = 32, skip = 3, drawn_segments = segments - skip;

		//  dir = norm3(dir);
		glm::vec3 right = tempMath::cross3(dir, glm::vec3(0, 1, 0));
		glm::vec3 up = tempMath::cross3(dir, right);
		right = tempMath::cross3(dir, up);

		glm::vec3 point, lastPoint;
		dir = tempMath::scale3(dir, radius);
		right = tempMath::scale3(right, radius);
		//lastPoint = tempMath::add3(center, dir);

		{
			const float radians = (C_PI * 2) * (0 + skip / 2.f) / segments;
			glm::vec3 vs = tempMath::scale3(right, sinf(radians));
			glm::vec3 vc = tempMath::scale3(dir, cosf(radians));
			lastPoint = tempMath::add3(center, vs);
			lastPoint = tempMath::add3(lastPoint, vc);
		}

		DrawLine(lastPoint, tempMath::add3(center, tempMath::scale3(dir, radius * 1.25)), rgb);

		for (int i = 0; i <= drawn_segments; ++i) 
		{
			const float radians = (C_PI * 2) * (i + skip / 2.f) / segments;

			glm::vec3 vs = tempMath::scale3(right, sinf(radians));
			glm::vec3 vc = tempMath::scale3(dir, cosf(radians));

			point = tempMath::add3(center, vs);
			point = tempMath::add3(point, vc);

			DrawLine(lastPoint, point, rgb);
			lastPoint = point;
		}

		DrawLine(lastPoint, tempMath::add3(center, tempMath::scale3(dir, radius * 1.25)), rgb);
	}
}

void DebugDraw::DrawPoint(const glm::vec3& from, unsigned rgb)
{
	Points[rgb].push_back(from);
}

void DebugDraw::DrawLine(const glm::vec3& from, const glm::vec3& to, unsigned rgb)
{
	Lines[rgb].push_back(from);
	Lines[rgb].push_back(to);
}

void DebugDraw::DrawLineDashed(glm::vec3 from, glm::vec3 to, unsigned rgb)
{
	glm::vec3 dist = tempMath::sub3(to, from); 
	glm::vec3 unit = tempMath::norm3(dist);
	for (float len = 0, mag = tempMath::len3(dist) / 2; len < mag; ++len)
	{
		to = tempMath::add3(from, unit);
		DrawLine(from, to, rgb);
		from = tempMath::add3(to, unit);
	}
}

void DebugDraw::DrawAxis(float units)
{
	DrawLine(glm::vec3(0, 0, 0), glm::vec3(units, 0, 0), RED);
	DrawLineDashed(glm::vec3(0, 0, 0), glm::vec3(-units, 0, 0), RED);

	DrawLine(glm::vec3(0, 0, 0), glm::vec3(0, units, 0), GREEN);
	DrawLineDashed(glm::vec3(0, 0, 0), glm::vec3(0, -units, 0), GREEN);

	DrawLine(glm::vec3(0, 0, 0), glm::vec3(0, 0, units), BLUE);
	DrawLineDashed(glm::vec3(0, 0, 0), glm::vec3(0, 0, -units), BLUE);
}

void DebugDraw::DrawGround(float scale)
{
	if (scale) 
	{
		drawGround_(scale);
	}
	else
	{
		drawGround_(100);
		drawGround_(10);
		drawGround_(1);
		drawGround_(0.1);
		drawGround_(0.01);
	}
}

void DebugDraw::DrawGrid(float scale)
{
	DrawGround(scale);
	DrawAxis(scale ? scale : 100.0f);
}

void DebugDraw::DrawTriangle(const glm::vec3& pa, const glm::vec3& pb, const glm::vec3& pc, unsigned rgb)
{
	DrawLine(pa, pb, rgb);
	DrawLine(pa, pc, rgb);
	DrawLine(pb, pc, rgb);
}

void DebugDraw::DrawArrow(const glm::vec3& begin, const glm::vec3& end, unsigned rgb)
{
	glm::vec3 diff = tempMath::sub3(end, begin);
	float len = tempMath::len3(diff), stick_len = len * 2 / 3;

	DrawLine(begin, end, rgb);
	DrawConeLowres(tempMath::add3(begin, tempMath::scale3(tempMath::norm3(diff), stick_len)), end, len / 6, rgb);
}

void DebugDraw::DrawBounds(const glm::vec3 points[8], unsigned rgb)
{
	for (int i = 0; i < 4; ++i) 
	{
		DrawLine(points[i], points[(i + 1) & 3], rgb);
		DrawLine(points[i], points[4 + i], rgb);
		DrawLine(points[4 + i], points[4 + ((i + 1) & 3)], rgb);
	}
}

void DebugDraw::DrawBox(const glm::vec3& c, const glm::vec3& extents, unsigned rgb)
{
	glm::vec3 points[8], whd = tempMath::scale3(extents, 0.5f);
#define DD_BOX_V(v, op1, op2, op3) (v).x = c.x op1 whd.x; (v).y = c.y op2 whd.y; (v).z = c.z op3 whd.z
	DD_BOX_V(points[0], -, +, +);
	DD_BOX_V(points[1], -, +, -);
	DD_BOX_V(points[2], +, +, -);
	DD_BOX_V(points[3], +, +, +);
	DD_BOX_V(points[4], -, -, +);
	DD_BOX_V(points[5], -, -, -);
	DD_BOX_V(points[6], +, -, -);
	DD_BOX_V(points[7], +, -, +);
#undef DD_BOX_V
	DrawBounds(points, rgb);
}

void DebugDraw::DrawCube(const glm::vec3& center, float radius, unsigned rgb)
{
	// draw_prism(center, 1, -1, vec3(0,1,0), 4);
	float half = radius * 0.5f;
	glm::vec3 l = glm::vec3(center.x - half, center.y + half, center.z - half); // left-top-far
	glm::vec3 r = glm::vec3(center.x + half, center.y - half, center.z + half); // right-bottom-near

	DrawLine(l, glm::vec3(r.x, l.y, l.z), rgb);
	DrawLine(glm::vec3(r.x, l.y, l.z), glm::vec3(r.x, l.y, r.z), rgb);
	DrawLine(glm::vec3(r.x, l.y, r.z), glm::vec3(l.x, l.y, r.z), rgb);
	DrawLine(glm::vec3(l.x, l.y, r.z), l, rgb);
	DrawLine(l, glm::vec3(l.x, r.y, l.z), rgb);

	DrawLine(r, glm::vec3(l.x, r.y, r.z), rgb);
	DrawLine(glm::vec3(l.x, r.y, r.z), glm::vec3(l.x, r.y, l.z), rgb);
	DrawLine(glm::vec3(l.x, r.y, l.z), glm::vec3(r.x, r.y, l.z), rgb);
	DrawLine(glm::vec3(r.x, r.y, l.z), r, rgb);
	DrawLine(r, glm::vec3(r.x, l.y, r.z), rgb);

	DrawLine(glm::vec3(l.x, l.y, r.z), glm::vec3(l.x, r.y, r.z), rgb);
	DrawLine(glm::vec3(r.x, l.y, l.z), glm::vec3(r.x, r.y, l.z), rgb);
}

void DebugDraw::DrawPlane(const glm::vec3& p, const glm::vec3& n, float scale, unsigned rgb)
{
	// if n is too similar to up vector, use right. else use up vector
	glm::vec3 v1 = tempMath::cross3(n, tempMath::dot3(n, glm::vec3(0, 1, 0)) > 0.8f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0));
	glm::vec3 v2 = tempMath::cross3(n, v1);

	// draw axis
	DrawLine(p, tempMath::add3(p, n), rgb);
	DrawLine(p, tempMath::add3(p, v1), rgb);
	DrawLine(p, tempMath::add3(p, v2), rgb);

	// get plane coords
	v1 = tempMath::scale3(v1, scale);
	v2 = tempMath::scale3(v2, scale);
	glm::vec3 p1 = tempMath::add3(tempMath::add3(p, v1), v2);
	glm::vec3 p2 = tempMath::add3(tempMath::sub3(p, v1), v2);
	glm::vec3 p3 = tempMath::sub3(tempMath::sub3(p, v1), v2);
	glm::vec3 p4 = tempMath::sub3(tempMath::add3(p, v1), v2);

	// draw plane
	DrawLine(p1, p2, rgb);
	DrawLine(p2, p3, rgb);
	DrawLine(p3, p4, rgb);
	DrawLine(p4, p1, rgb);
}

void DebugDraw::DrawSphere(const glm::vec3& center, float radius, unsigned rgb)
{
	float lod = 6, yp = -radius, rp = 0, y, r, x, z;
	for (int j = 1; j <= lod / 2; ++j, yp = y, rp = r) 
	{
		y = j * 2.f / (lod / 2) - 1;
		r = cosf(y * 3.14159f / 2) * radius;
		y = sinf(y * 3.14159f / 2) * radius;

		float xp = 1, zp = 0;
		for (int i = 1; i <= lod; ++i, xp = x, zp = z)
		{
			x = 3.14159f * 2 * i / lod;
			z = sinf(x);
			x = cosf(x);

			glm::vec3 a1 = tempMath::add3(center, glm::vec3(xp * rp, yp, zp * rp));
			glm::vec3 b1 = tempMath::add3(center, glm::vec3(xp * r, y, zp * r));
			glm::vec3 c1 = tempMath::add3(center, glm::vec3(x * r, y, z * r));

			DrawLine(a1, b1, rgb);
			DrawLine(b1, c1, rgb);
			DrawLine(c1, a1, rgb);

			glm::vec3 a2 = tempMath::add3(center, glm::vec3(xp * rp, yp, zp * rp));
			glm::vec3 b2 = tempMath::add3(center, glm::vec3(x * r, y, z * r));
			glm::vec3 c2 = tempMath::add3(center, glm::vec3(x * rp, yp, z * rp));

			DrawLine(a2, b2, rgb);
			DrawLine(b2, c2, rgb);
			DrawLine(c2, a2, rgb);
		}
	}
}

void DebugDraw::DrawCapsule(const glm::vec3& from, const glm::vec3& to, float r, unsigned rgb)
{
	/* calculate axis */
	glm::vec3 up, right, forward;
	forward = tempMath::sub3(to, from);
	forward = tempMath::norm3(forward);
	tempMath::ortho3(&right, &up, forward);

	/* calculate first two cone verts (buttom + top) */
	glm::vec3 lastf, lastt;
	lastf = tempMath::scale3(up, r);
	lastt = tempMath::add3(to, lastf);
	lastf = tempMath::add3(from, lastf);

	/* step along circle outline and draw lines */
	enum { step_size = 20 };
	for (int i = step_size; i <= 360; i += step_size)
	{
		/* calculate current rotation */
		glm::vec3 ax = tempMath::scale3(right, sinf(i * TO_RAD));
		glm::vec3 ay = tempMath::scale3(up, cosf(i * TO_RAD));

		/* calculate current vertices on cone */
		glm::vec3 tmp = tempMath::add3(ax, ay);
		glm::vec3 pf = tempMath::scale3(tmp, r);
		glm::vec3 pt = tempMath::scale3(tmp, r);

		pf = tempMath::add3(pf, from);
		pt = tempMath::add3(pt, to);

		/* draw cone vertices */
		DrawLine(lastf, pf, rgb);
		DrawLine(lastt, pt, rgb);
		DrawLine(pf, pt, rgb);

		lastf = pf;
		lastt = pt;

		/* calculate first top sphere vert */
		glm::vec3 prevt = tempMath::scale3(tmp, r);
		glm::vec3 prevf = tempMath::add3(prevt, from);
		prevt = tempMath::add3(prevt, to);

		/* sphere (two half spheres )*/
		for (int j = 1; j < 180 / step_size; j++) 
		{
			/* angles */
			float ta = j * step_size;
			float fa = 360 - (j * step_size);

			/* top half-sphere */
			ax = tempMath::scale3(forward, sinf(ta * TO_RAD));
			ay = tempMath::scale3(tmp, cosf(ta * TO_RAD));

			glm::vec3 t = tempMath::add3(ax, ay);
			pf = tempMath::scale3(t, r);
			pf = tempMath::add3(pf, to);
			DrawLine(pf, prevt, rgb);
			prevt = pf;

			/* bottom half-sphere */
			ax = tempMath::scale3(forward, sinf(fa * TO_RAD));
			ay = tempMath::scale3(tmp, cosf(fa * TO_RAD));

			t = tempMath::add3(ax, ay);
			pf = tempMath::scale3(t, r);
			pf = tempMath::add3(pf, from);
			DrawLine(pf, prevf, rgb);
			prevf = pf;
		}
	}
}

void DebugDraw::DrawDiamond(const glm::vec3& from, const glm::vec3& to, float size, unsigned rgb)
{
	Poly p = collide::Diamond(from, to, size);
	glm::vec3* dmd = p.verts.data();

	glm::vec3* a = dmd + 0;
	glm::vec3* b = dmd + 1;
	glm::vec3* c = dmd + 2;
	glm::vec3* d = dmd + 3;
	glm::vec3* t = dmd + 4;
	glm::vec3* f = dmd + 5;

	/* draw vertices */
	DrawLine(*a, *b, rgb);
	DrawLine(*b, *c, rgb);
	DrawLine(*c, *d, rgb);
	DrawLine(*d, *a, rgb);

	/* draw roof */
	DrawLine(*a, *t, rgb);
	DrawLine(*b, *t, rgb);
	DrawLine(*c, *t, rgb);
	DrawLine(*d, *t, rgb);

	/* draw floor */
	DrawLine(*a, *f, rgb);
	DrawLine(*b, *f, rgb);
	DrawLine(*c, *f, rgb);
	DrawLine(*d, *f, rgb);
}

void DebugDraw::DrawPyramid(const glm::vec3& center, float height, int segments, unsigned rgb)
{
	DrawPrism(center, 1, height, glm::vec3(0, 1, 0), segments, rgb);
}

void DebugDraw::DrawPrism(const glm::vec3& center, float radius, float height, const glm::vec3& normal, int segments, unsigned rgb)
{
	glm::vec3 left = glm::vec3{ 0 }, up = glm::vec3{ 0 };
	tempMath::ortho3(&left, &up, normal);

	glm::vec3 point, lastPoint;
	up = tempMath::scale3(up, radius);
	left = tempMath::scale3(left, radius);
	lastPoint = tempMath::add3(center, up);
	glm::vec3 pivot = tempMath::add3(center, tempMath::scale3(normal, height));

	for (int i = 1; i <= segments; ++i)
	{
		const float radians = (C_PI * 2) * i / segments;

		glm::vec3 vs = tempMath::scale3(left, sinf(radians));
		glm::vec3 vc = tempMath::scale3(up, cosf(radians));

		point = tempMath::add3(center, vs);
		point = tempMath::add3(point, vc);

		DrawLine(lastPoint, point, rgb);
		if (height > 0) 
			DrawLine(point, pivot, rgb);
		else if (height < 0) 
		{
			DrawLine(point, tempMath::add3(point, tempMath::scale3(normal, -height)), rgb);
		}
		lastPoint = point;
	}

	if (height < 0) 
		DrawPrism(tempMath::add3(center, tempMath::scale3(normal, -height)), radius, 0, normal, segments, rgb);
}

void DebugDraw::DrawSquare(const glm::vec3& pos, float radius, unsigned rgb)
{
	DrawPrism(pos, radius, 0, glm::vec3(0, 1, 0), 4, rgb);
}

void DebugDraw::DrawCylinder(const glm::vec3& center, float height, int segments, unsigned rgb)
{
	DrawPrism(center, 1, -height, glm::vec3(0, 1, 0), segments, rgb);
}

void DebugDraw::DrawPentagon(const glm::vec3& pos, float radius, unsigned rgb)
{
	DrawPrism(pos, radius, 0, glm::vec3(0, 1, 0), 5, rgb);
}

void DebugDraw::DrawHexagon(const glm::vec3& pos, float radius, unsigned rgb)
{
	DrawPrism(pos, radius, 0, glm::vec3(0, 1, 0), 6, rgb);
}

void DebugDraw::DrawCone(const glm::vec3& center, const glm::vec3& top, float radius, unsigned rgb)
{
	glm::vec3 diff3 = tempMath::sub3(top, center);
	DrawPrism(center, radius ? radius : 1, tempMath::len3(diff3), tempMath::norm3(diff3), 24, rgb);
}

void DebugDraw::DrawCircle(const glm::vec3& pos, const glm::vec3& n, float radius, unsigned rgb)
{
	DrawPrism(pos, radius, 0, n, 32, rgb);
}

void DebugDraw::DrawAABB(const glm::vec3& minbb, const glm::vec3& maxbb, unsigned rgb)
{
	glm::vec3 points[8], bb[2] = { minbb, maxbb };
	for (int i = 0; i < 8; ++i) 
	{
		points[i].x = bb[(i ^ (i >> 1)) & 1].x;
		points[i].y = bb[(i >> 1) & 1].y;
		points[i].z = bb[(i >> 2) & 1].z;
	}
	DrawBounds/*_corners*/(points, rgb);
}

void DebugDraw::DrawPosition(const glm::vec3& pos, float radius)
{
	DrawPositionDir(pos, glm::vec3(0, 0, 0), radius);
}

void DebugDraw::DrawPositionDir(const glm::vec3& position, const glm::vec3& direction, float radius)
{
	// idea from http://www.cs.caltech.edu/~keenan/m3drv.pdf and flotilla game UI

	glm::vec3 ground = glm::vec3(position.x, 0, position.z);
	unsigned clr = position.y < 0 ? PINK/*ORANGE*/ : CYAN;

	DrawPoint(ground, clr);
	DrawPoint(position, clr);
	(position.y < 0 ? DrawLineDashed(ground, position, clr) : DrawLine(ground, position, clr));

	glm::vec3 n = tempMath::norm3(direction), up = glm::vec3(0, 1, 0);
	for (int i = 0; i < 10 && i <= fabs(position.y); ++i)
	{
		if (i < 2 && tempMath::len3(direction))
			DrawCircleWithOrientation(ground, n, radius, clr);
		else
			DrawCircle(ground, up, radius, clr);
		radius *= 0.9f;
	}
}

void DebugDraw::DrawNormal(const glm::vec3& pos, const glm::vec3& n)
{
	DrawLine(pos, tempMath::add3(pos, tempMath::norm3(n)), YELLOW);
}

void DebugDraw::DrawBone(const glm::vec3& center, const glm::vec3& end, unsigned rgb)
{
	glm::vec3 diff3 = tempMath::sub3(end, center);
	float len = tempMath::len3(diff3), len10 = len / 10;
	DrawPrism(center, len10, 0, glm::vec3(1, 0, 0), 24, rgb);
	DrawPrism(center, len10, 0, glm::vec3(0, 1, 0), 24, rgb);
	DrawPrism(center, len10, 0, glm::vec3(0, 0, 1), 24, rgb);
	DrawLine(end, tempMath::add3(center, glm::vec3(0, +len10, 0)), rgb);
	DrawLine(end, tempMath::add3(center, glm::vec3(0, -len10, 0)), rgb);
}

void DebugDraw::DrawBoid(const glm::vec3& position, glm::vec3 dir)
{
	dir = tempMath::norm3(dir);

	// if n is too similar to up vector, use right. else use up vector
	glm::vec3 v1 = tempMath::cross3(dir, tempMath::dot3(dir, glm::vec3(0, 1, 0)) > 0.8f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0));
	glm::vec3 v2 = tempMath::cross3(dir, v1);
	v1 = tempMath::cross3(dir, v2);

	uint32_t clr = position.y < 0 ? ORANGE : CYAN;

	glm::vec3 front = tempMath::add3(position, tempMath::scale3(dir, 1));
	glm::vec3 back = tempMath::add3(position, tempMath::scale3(dir, -0.25f));
	glm::vec3 right = tempMath::add3(back, tempMath::scale3(v1, 0.5f));
	glm::vec3 left = tempMath::add3(back, tempMath::scale3(v1, -0.5f));
	DrawLine(front, left, clr);
	DrawLine(left, position, clr);
	DrawLine(position, right, clr);
	DrawLine(right, front, clr);
}

void DebugDraw::Flush(const Camera& camera)
{
	if (Points.empty() && Lines.empty())
		return;

	static bool isCreate = false;
	static ShaderProgram shaderProgram;
	static UniformLocation MatrixID;
	static UniformLocation ColorID;
	static GLuint vao, vbo;
	if (!isCreate)
	{
		isCreate = true;

		const char* vertexSource = R"(
#version 330 core
layout(location = 0) in vec3 vertexPosition;
uniform mat4 MVP;
uniform vec3 u_color;
out vec3 out_color;
void main()
{
	gl_Position =  MVP * vec4(vertexPosition, 1);
	out_color = u_color;
}
)";

		const char* fragmentSource = R"(
#version 330 core
in vec3 out_color;
out vec4 fragcolor;
void main()
{
	fragcolor = vec4(out_color, 1.0);
}
)";
		shaderProgram.CreateFromMemories(vertexSource, fragmentSource);
		shaderProgram.Bind();
		MatrixID = shaderProgram.GetUniformVariable("MVP");
		ColorID = shaderProgram.GetUniformVariable("u_color");

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
	}

	const glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.m_view;
	shaderProgram.Bind();
	shaderProgram.SetUniform(MatrixID, MVP);

	// TODO: сделать интерфейс
	VertexArrayBuffer::UnBind();
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_PROGRAM_POINT_SIZE); // for GL_POINTS
	glEnable(GL_LINE_SMOOTH); // for GL_LINES (thin)
	
	// Draw Points
	{
		glPointSize(6);
		for (auto& it : Points)
		{
			shaderProgram.SetUniform(ColorID, rgbf(it.first));
			const size_t count = it.second.size();
			glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec3), it.second.data(), GL_STATIC_DRAW);
			glDrawArrays(GL_POINTS, 0, count);
		}
		glPointSize(1);
	}

	//glDisable(GL_DEPTH_TEST);
	// Draw Lines
	{
		for (auto& it : Lines)
		{
			shaderProgram.SetUniform(ColorID, rgbf(it.first));
			const size_t count = it.second.size();
			glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec3), it.second.data(), GL_STATIC_DRAW);
			glDrawArrays(GL_LINES, 0, count);
		}
	}
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_PROGRAM_POINT_SIZE);
	glBindVertexArray(0);

	Points.clear();
	Lines.clear();
}

namespace std
{
	template <>
	struct hash<Vertex_Pos3_TexCoord>
	{
		size_t operator()(const Vertex_Pos3_TexCoord& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1);
		}
	};
} // namespace std


#pragma region Graphics3D
namespace g3d
{

	void FreeCamera::MoveForward(float deltaTime, float speedMod)
	{
		m_position += m_front * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::MoveBackward(float deltaTime, float speedMod)
	{
		m_position -= m_front * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::MoveRight(float deltaTime, float speedMod)
	{
		m_position -= m_right * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::MoveLeft(float deltaTime, float speedMod)
	{
		m_position += m_right * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::MoveUp(float deltaTime, float speedMod)
	{
		m_position += m_up * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::MoveDown(float deltaTime, float speedMod)
	{
		m_position -= m_up * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::Rotate(float offsetX, float offsetY)
	{
		m_yaw -= offsetX * m_sensitivity;
		m_pitch += offsetY * m_sensitivity;
		SetRotate(m_yaw, m_pitch);
	}

	void FreeCamera::SetRotate(float yaw, float pitch)
	{
		m_yaw = yaw;
		m_pitch = pitch;
		if (m_pitch > 89.0f) m_pitch = 89.0f;
		else if (m_pitch < -89.0f) m_pitch = -89.0f;
		if (m_yaw > 360.0f) m_yaw = 0.0f;
		else if (m_yaw < -360.0f) m_yaw = 0.0f;
		updateVectors();
	}

	void FreeCamera::SimpleMove(float deltaTime)
	{
		const float xpos = GetMouseX();
		const float ypos = GetMouseY();
		static float lastPosX = xpos;
		static float lastPosY = ypos;
		Rotate((xpos - lastPosX), (lastPosY - ypos));
		lastPosX = xpos;
		lastPosY = ypos;

		constexpr float speedMod = 1.0f;
		if (IsKeyboardKeyDown(KEY_W))
			MoveForward(deltaTime, speedMod);
		if (IsKeyboardKeyDown(KEY_S))
			MoveBackward(deltaTime, speedMod);
		if (IsKeyboardKeyDown(KEY_D))
			MoveRight(deltaTime, speedMod);
		if (IsKeyboardKeyDown(KEY_A))
			MoveLeft(deltaTime, speedMod);

		constexpr float speedRotateMod = 1600.0f;
		if (IsKeyboardKeyDown(KEY_E))
			Rotate(speedRotateMod * deltaTime, 0.0f);
		if (IsKeyboardKeyDown(KEY_Q))
			Rotate(-speedRotateMod * deltaTime, 0.0f);

#ifdef _DEBUG
		if (IsKeyboardKeyDown(KEY_T))
			MoveUp(deltaTime, speedMod / 2.0f);
		if (IsKeyboardKeyDown(KEY_G))
			MoveDown(deltaTime, speedMod / 2.0f);

		if (IsKeyboardKeyDown(KEY_R))
			Rotate(0.0f, speedRotateMod * deltaTime);
		if (IsKeyboardKeyDown(KEY_F))
			Rotate(0.0f, -speedRotateMod * deltaTime);
#endif

		Update();
	}

	void FreeCamera::Update()
	{
		m_viewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
	}

	//Frustum FreeCamera::ComputeFrustum() const
	//{
	//	Frustum frustum;

	//	const float halfVSide = m_far * tanf(m_fov * 0.5f);
	//	const float halfHSide = halfVSide * GetWindowAspect();
	//	const glm::vec3 frontMultFar = m_far * m_front;

	//	frustum.nearFace = { m_position + m_near * m_front, m_front };
	//	frustum.farFace = { m_position + frontMultFar, -m_front };
	//	frustum.rightFace = { m_position, glm::cross(m_up, frontMultFar + m_right * halfHSide) };
	//	frustum.leftFace = { m_position, glm::cross(frontMultFar - m_right * halfHSide, m_up) };
	//	frustum.topFace = { m_position, glm::cross(m_right, frontMultFar - m_up * halfVSide) };
	//	frustum.bottomFace = { m_position, glm::cross(frontMultFar + m_up * halfVSide, m_right) };

	//	return frustum;
	//}

	void FreeCamera::updateVectors()
	{
		const float radiansYaw = glm::radians(m_yaw);
		const float radiansPitch = glm::radians(m_pitch);

		const glm::vec3 front = {
			cos(radiansYaw) * cos(radiansPitch),
			sin(radiansPitch),
			sin(radiansYaw) * cos(radiansPitch)
		};
		m_front = glm::normalize(front);
		m_right = glm::normalize(glm::cross(m_front, m_worldUp));
		m_up = glm::normalize(glm::cross(m_right, m_front));
	}

	bool Model::Create(const char* fileName, const char* pathMaterialFiles)
	{
		Destroy();

		tinyobj::ObjReaderConfig readerConfig;
		readerConfig.mtl_search_path = pathMaterialFiles; // Path to material files

		tinyobj::ObjReader reader;
		if (!reader.ParseFromFile(fileName, readerConfig))
		{
			if (!reader.Error().empty())
				LogError("TinyObjReader: " + reader.Error());
			return false;
		}
		if (!reader.Warning().empty())
			LogWarning("TinyObjReader: " + reader.Warning());

		auto& attributes = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		const bool isFindMaterials = !materials.empty();

		std::vector<Mesh> tempMesh(materials.size());
		std::vector<std::unordered_map<Vertex_Pos3_TexCoord, uint32_t>> uniqueVertices(materials.size());
		if (tempMesh.empty())
		{
			tempMesh.resize(1);
			uniqueVertices.resize(1);
		}

		// Loop over shapes
		for (size_t shapeId = 0; shapeId < shapes.size(); shapeId++)
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t faceId = 0; faceId < shapes[shapeId].mesh.num_face_vertices.size(); faceId++)
			{
				const size_t fv = static_cast<size_t>(shapes[shapeId].mesh.num_face_vertices[faceId]);

				// per-face material
				int materialId = shapes[shapeId].mesh.material_ids[faceId];
				if (materialId < 0) materialId = 0;

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					// access to vertex
					const tinyobj::index_t idx = shapes[shapeId].mesh.indices[index_offset + v];

					// vertex position
					const tinyobj::real_t vx = attributes.vertices[3 * size_t(idx.vertex_index) + 0];
					const tinyobj::real_t vy = attributes.vertices[3 * size_t(idx.vertex_index) + 1];
					const tinyobj::real_t vz = attributes.vertices[3 * size_t(idx.vertex_index) + 2];

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0)
					{
						tinyobj::real_t nx = attributes.normals[3 * size_t(idx.normal_index) + 0];
						tinyobj::real_t ny = attributes.normals[3 * size_t(idx.normal_index) + 1];
						tinyobj::real_t nz = attributes.normals[3 * size_t(idx.normal_index) + 2];
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					tinyobj::real_t tx = 0;
					tinyobj::real_t ty = 0;
					if (idx.texcoord_index >= 0)
					{
						tx = attributes.texcoords[2 * size_t(idx.texcoord_index) + 0];
						ty = attributes.texcoords[2 * size_t(idx.texcoord_index) + 1];
					}

					// vertex colors
					const tinyobj::real_t r = attributes.colors[3 * size_t(idx.vertex_index) + 0];
					const tinyobj::real_t g = attributes.colors[3 * size_t(idx.vertex_index) + 1];
					const tinyobj::real_t b = attributes.colors[3 * size_t(idx.vertex_index) + 2];

					glm::vec3 position{ vx, vy, vz };
					glm::vec2 texCoord{ tx,ty };
					Vertex_Pos3_TexCoord vertex{ position, texCoord };

					if (uniqueVertices[materialId].count(vertex) == 0)
					{
						uniqueVertices[materialId][vertex] = static_cast<uint32_t>(tempMesh[materialId].vertices.size());
						tempMesh[materialId].vertices.emplace_back(vertex);
					}

					tempMesh[materialId].indices.emplace_back(uniqueVertices[materialId][vertex]);
				}
				index_offset += fv;
			}
		}

		// load materials
		bool isFindToTransparent = false;
		if (isFindMaterials)
		{
			for (int i = 0; i < materials.size(); i++)
			{
				if (materials[i].diffuse_texname.empty()) continue;

				std::string diffuseMap = pathMaterialFiles + materials[i].diffuse_texname;
				tempMesh[i].material.diffuseTexture = TextureLoader::LoadTexture2D(diffuseMap.c_str());
				if (!isFindToTransparent && tempMesh[i].material.diffuseTexture)
					isFindToTransparent = tempMesh[i].material.diffuseTexture->isTransparent;
			}
		}

		// сортировка по прозрачности
		if (isFindToTransparent)
		{
			std::vector<Mesh> tempMesh2;

			// TODO: медленно, оптимизировать

			// сначала непрозрачное
			for (int i = 0; i < tempMesh.size(); i++)
			{
				if (!tempMesh[i].material.diffuseTexture)
					tempMesh2.push_back(tempMesh[i]);
				else if (!tempMesh[i].material.diffuseTexture->isTransparent)
					tempMesh2.push_back(tempMesh[i]);
			}
			// теперь прозрачное
			for (int i = 0; i < tempMesh.size(); i++)
			{
				if (tempMesh[i].material.diffuseTexture->isTransparent)
					tempMesh2.push_back(tempMesh[i]);
			}

			m_subMeshes = std::move(tempMesh2);
		}
		else
			m_subMeshes = std::move(tempMesh);

		return createBuffer();
	}

	bool g3d::Model::Create(std::vector<MeshCreateInfo>&& meshes)
	{
		Destroy();
		m_subMeshes.resize(meshes.size());
		for (int i = 0; i < meshes.size(); i++)
			m_subMeshes[i].Set(std::move(meshes[i]));

		return createBuffer();
	}

	void Model::Destroy()
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			m_subMeshes[i].vertices.clear();
			m_subMeshes[i].indices.clear();

			m_subMeshes[i].vertexBuffer.Destroy();
			m_subMeshes[i].indexBuffer.Destroy();
			m_subMeshes[i].vao.Destroy();
		}
		m_subMeshes.clear();
	}

	void Model::SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttributeRaw>& attribs)
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			if (m_subMeshes[i].vao.IsValid())
				m_subMeshes[i].vao.SetInstancedBuffer(instanceBuffer, attribs);
		}
	}

	void Model::Draw(uint32_t instanceCount)
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			if (m_subMeshes[i].vao.IsValid())
			{
				const Texture2D* diffuseTexture = m_subMeshes[i].material.diffuseTexture;
				if (diffuseTexture && diffuseTexture->IsValid())
					diffuseTexture->Bind(0);
				m_subMeshes[i].vao.Draw(PrimitiveDraw::Triangles, instanceCount);
			}
		}
	}

	void Model::SetMaterial(const Material& material)
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			m_subMeshes[i].material = material;
		}
	}

	bool Model::createBuffer()
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			if (!m_subMeshes[i].vertexBuffer.Create(RenderResourceUsage::Static, m_subMeshes[i].vertices.size(), sizeof(m_subMeshes[i].vertices[0]), m_subMeshes[i].vertices.data()))
			{
				LogError("VertexBuffer create failed!");
				Destroy();
				return false;
			}
			if (!m_subMeshes[i].indexBuffer.Create(RenderResourceUsage::Static, m_subMeshes[i].indices.size(), sizeof(uint32_t), m_subMeshes[i].indices.data()))
			{
				LogError("IndexBuffer create failed!");
				Destroy();
				return false;
			}

			if (!m_subMeshes[i].vao.Create<Vertex_Pos3_TexCoord>(&m_subMeshes[i].vertexBuffer, &m_subMeshes[i].indexBuffer))
			{
				LogError("VAO create failed!");
				Destroy();
				return false;
			}
		}
		return true;
	}

	namespace ModelFileManager
	{
		std::unordered_map<std::string, Model> FileModels;

		void Destroy()
		{
			for (auto it = FileModels.begin(); it != FileModels.end(); ++it)
				it->second.Destroy();
			FileModels.clear();
		}

		Model* LoadModel(const char* name)
		{
			auto it = FileModels.find(name);
			if (it != FileModels.end())
			{
				return &it->second;
			}
			else
			{
				LogPrint("Load model: " + std::string(name));

				Model model;
				if (!model.Create(name) || !model.IsValid())
					return nullptr;

				FileModels[name] = model;
				return &FileModels[name];
			}
		}
	}

	void drawPrimitive::DrawLine(const FreeCamera& camera, const glm::vec3& startPos, const glm::vec3& endPos)
	{
		static bool isCreate = false;
		static VertexArrayBuffer vao;
		static VertexBuffer vertexBuf;
		static ShaderProgram shaderProgram;
		static UniformLocation MatrixID;

		if (!isCreate)
		{
			isCreate = true;

			const float vertexData[] =
			{
				startPos.x, startPos.y, startPos.z,// 0
				endPos.x, endPos.y,  endPos.z// 1
			};

			vertexBuf.Create(RenderResourceUsage::Dynamic, 2, 3 * sizeof(float), vertexData);

			const std::vector<VertexAttributeRaw> attribs =
			{
				{.size = 3, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = 0, .pointer = (void*)0},
			};
			vao.Create(&vertexBuf, nullptr, attribs);

			const char* vertexSource = R"(
#version 330 core
layout(location = 0) in vec3 vertexPosition;
uniform mat4 MVP;
void main()
{
gl_Position =  MVP * vec4(vertexPosition, 1);
}
)";

			const char* fragmentSource = R"(
#version 330 core
out vec4 outColor;
void main()
{
outColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";
			shaderProgram.CreateFromMemories(vertexSource, fragmentSource);
			shaderProgram.Bind();
			MatrixID = shaderProgram.GetUniformVariable("MVP");
		}
		else
		{
			const float vertexData[] =
			{
				startPos.x, startPos.y, startPos.z,// 0
				endPos.x, endPos.y,  endPos.z// 1
			};
			vertexBuf.Update(0, sizeof(vertexData), vertexData);
		}

		//const glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), position);
		const glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix()/* * worldMModelMatrixatrix*/;

		shaderProgram.Bind();
		shaderProgram.SetUniform(MatrixID, MVP);
		glLineWidth(4);
		glDisable(GL_DEPTH_TEST);
		vao.Draw(PrimitiveDraw::Lines);
		glLineWidth(1);
		glEnable(GL_DEPTH_TEST);
	}

	void drawPrimitive::DrawCubeWires(const FreeCamera& camera, const glm::mat4& worldMatrix, const glm::vec4& color, bool disableDepthTest)
	{
		static bool isCreate = false;
		static VertexArrayBuffer vao;
		static VertexBuffer vertexBuf;
		static IndexBuffer indexBuf;
		static ShaderProgram shaderProgram;
		static UniformLocation MatrixID;
		static UniformLocation ColorID;

		if (!isCreate)
		{
			isCreate = true;

			constexpr float vertexData[] =
			{
				-0.5f, -0.5f,  0.5f,// 0
					0.5f, -0.5f,  0.5f,// 1
					0.5f,  0.5f,  0.5f,// 2
				-0.5f,  0.5f,  0.5f,// 3
				-0.5f, -0.5f, -0.5f,// 4
					0.5f, -0.5f, -0.5f,// 5
					0.5f,  0.5f, -0.5f,// 6
				-0.5f,  0.5f, -0.5f,// 7
			};
			constexpr uint16_t indexData[] =
			{
				0, 1,
				1, 2,
				2, 3,
				3, 0,
				4, 5,
				5, 6,
				6, 7,
				7, 4,
				3, 7,
				2, 6,
				0, 4,
				1, 5
			};

			vertexBuf.Create(RenderResourceUsage::Static, 8, 3 * sizeof(float), vertexData);
			indexBuf.Create(RenderResourceUsage::Static, 24, sizeof(uint16_t), indexData);

			const std::vector<VertexAttributeRaw> attribs =
			{
				{.size = 3, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = 0, .pointer = (void*)0},
			};
			vao.Create(&vertexBuf, &indexBuf, attribs);

			const char* vertexSource = R"(
#version 330 core
layout(location = 0) in vec3 vertexPosition;
uniform mat4 MVP;
void main()
{
gl_Position =  MVP * vec4(vertexPosition, 1);
}
)";

			const char* fragmentSource = R"(
#version 330 core
uniform vec4 inColor;
out vec4 outColor;
void main()
{
outColor = inColor;
}
)";
			shaderProgram.CreateFromMemories(vertexSource, fragmentSource);
			shaderProgram.Bind();
			MatrixID = shaderProgram.GetUniformVariable("MVP");
			ColorID = shaderProgram.GetUniformVariable("inColor");
		}

		const glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix() * worldMatrix;

		shaderProgram.Bind();
		shaderProgram.SetUniform(MatrixID, MVP);
		shaderProgram.SetUniform(ColorID, color);
		if (disableDepthTest) glDisable(GL_DEPTH_TEST);
		//glLineWidth(4);
		vao.Draw(PrimitiveDraw::Lines);
		if (disableDepthTest) glEnable(GL_DEPTH_TEST);
	}

	void drawPrimitive::DrawCubeWires(const FreeCamera& camera, const glm::vec3& position, const glm::vec3& size, const glm::vec3& rotationRadian, const glm::vec4& color, bool disableDepthTest)
	{
		const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f), rotationRadian.x, glm::vec3(1.0f, 0.0f, 0.0f));
		const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f), rotationRadian.y, glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f), rotationRadian.z, glm::vec3(0.0f, 0.0f, 1.0f));
		const glm::mat4 roationMatrix = transformY * transformX * transformZ;

		// translation * rotation * scale (also know as TRS matrix)
		const glm::mat4 ModelMatrix =
			glm::translate(glm::mat4(1.0f), position)
			* roationMatrix
			* glm::scale(glm::mat4(1.0f), size);

		drawPrimitive::DrawCubeWires(camera, ModelMatrix, color, disableDepthTest);
	}

}
#pragma endregion

#pragma region Graphics2D
namespace g2d
{

	class Font
	{
	public:
		uint32_t size = 60;
		std::string fontFileName = "../fonts/OpenSans-Regular.ttf";
		const uint32_t atlasWidth = 1024;
		const uint32_t atlasHeight = 1024;
		//const uint32_t oversampleX = 2;
		//const uint32_t oversampleY =2;
		const uint32_t firstCharENG = ' ';
		const uint32_t charCountENG = '~' - ' ' + 1;
		const uint32_t firstCharRUS = 0x400;
		const uint32_t charCountRUS = 0x452 - 0x400;

		std::unique_ptr<stbtt_packedchar[]> charInfo;
		Texture2D texture;
	};

	static std::vector<Font> m_cacheFont;
	static ShaderProgram cacheShader;
	static UniformLocation m_idAttributeTextColor;
	static UniformLocation m_idAttributeWorldViewProjMatrix;

	constexpr const char* fontVertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

uniform mat4 worldViewProjMatrix;

out vec2 uv0;

void main()
{
    gl_Position = worldViewProjMatrix * vec4(vertex.xy, 0.0, 1.0);
    uv0 = vertex.zw;
}
)";
	constexpr const char* fontFragmentShaderSource = R"(
#version 330 core

in vec2 uv0;

uniform sampler2D mainTex;
uniform vec3 textColor;

out vec4 fragColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(mainTex, uv0).r);
    fragColor = vec4(textColor, 1.0) * sampled;
}
)";

	struct GlyphInfo
	{
		glm::vec4 positions[4];
		float offsetX = 0;
		float offsetY = 0;
	};

	inline GlyphInfo makeGlyphInfo(Font* font, uint32_t character, float offsetX, float offsetY)
	{
		stbtt_aligned_quad quad;

		int char_index = 0;
		if (character < font->firstCharENG + font->charCountENG)
			char_index = character - font->firstCharENG;
		else
			char_index = character - font->firstCharRUS + font->charCountENG;

		stbtt_GetPackedQuad(font->charInfo.get(), font->atlasWidth, font->atlasHeight, char_index, &offsetX, &offsetY, &quad, 1);

		const int sizeY = font->size / 2;

		GlyphInfo info{};
		info.offsetX = offsetX;
		info.offsetY = offsetY;
		info.positions[0] = { quad.x0, quad.y0 + sizeY, quad.s0, quad.t0 };
		info.positions[1] = { quad.x0, quad.y1 + sizeY, quad.s0, quad.t1 };
		info.positions[2] = { quad.x1, quad.y1 + sizeY, quad.s1, quad.t1 };
		info.positions[3] = { quad.x1, quad.y0 + sizeY, quad.s1, quad.t0 };

		return info;
	}

	Font* getFont(const std::string& fontFileName, uint32_t fontSize)
	{
		Font* font = nullptr;
		for (int i = 0; i < m_cacheFont.size(); i++)
		{
			if (m_cacheFont[i].fontFileName == fontFileName && m_cacheFont[i].size == fontSize)
			{
				font = &m_cacheFont[i];
				break;
			}
		}
		if (!font)
		{
			Font font_;
			font_.size = fontSize;
			font_.fontFileName = fontFileName;

			std::ifstream file(fontFileName, std::ios::binary | std::ios::ate);
			if (!file.is_open())
			{
				LogError("Failed to open file " + fontFileName);
				return nullptr;
			}

			const auto size = file.tellg();
			file.seekg(0, std::ios::beg);
			auto bytes = std::vector<uint8_t>(size);
			file.read(reinterpret_cast<char*>(&bytes[0]), size);
			file.close();

			auto atlasData = new uint8_t[font_.atlasWidth * font_.atlasHeight];

			font_.charInfo = std::make_unique<stbtt_packedchar[]>(font_.charCountENG + font_.charCountRUS);

			stbtt_pack_context context;
			//if (!stbtt_PackBegin(&context, atlasData.get(), font_.atlasWidth, font_.atlasHeight, 0, 1, nullptr))
			//	panic("Failed to initialize font");
			stbtt_PackBegin(&context, atlasData, font_.atlasWidth, font_.atlasHeight, 0, 1, nullptr);

			//stbtt_PackSetOversampling(&context, font_.oversampleX, font_.oversampleY);
			//if (!stbtt_PackFontRange(&context, fontData.data(), 0, font_.size, font_.firstChar, font_.charCount, font_.charInfo.get()))
			//    panic("Failed to pack font");

			//stbtt_PackFontRange(&context, fontData.data(), 0, font_.size, font_.firstChar, font_.charCount, font_.charInfo.get());
			stbtt_PackFontRange(&context, bytes.data(), 0, font_.size, font_.firstCharENG, font_.charCountENG, font_.charInfo.get());
			stbtt_PackFontRange(&context, bytes.data(), 0, font_.size, font_.firstCharRUS, font_.charCountRUS, font_.charInfo.get() + font_.charCountENG);

			stbtt_PackEnd(&context);

			Texture2DCreateInfo createInfo;
			createInfo.format = TexelsFormat::R_U8;
			createInfo.width = font_.atlasWidth;
			createInfo.height = font_.atlasHeight;
			createInfo.depth = 1;
			createInfo.pixelData = atlasData;			

			Texture2DInfo textureInfo;
			//textureInfo.minFilter = TextureMinFilter::Linear;
			//textureInfo.magFilter = TextureMagFilter::Linear;
			textureInfo.mipmap = false;

			//font_.texture.Create(GL_RGB, GL_RED, GL_UNSIGNED_BYTE, font_.atlasWidth, font_.atlasHeight, atlasData.get());
			font_.texture.Create(createInfo, textureInfo);

			delete[] atlasData;

			m_cacheFont.push_back(std::move(font_));
			font = &m_cacheFont[m_cacheFont.size() - 1];
		}

		return font;
	}

	bool Text::Create(const std::string& fontFileName, uint32_t fontSize)
	{
		Font* font = getFont(fontFileName, fontSize);
		if (!font || !create(font))
		{
			LogError("Text not create!");
			return false;
		}

		return true;
	}

	void Text::Destroy()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);
	}

	void Text::SetText(const std::wstring& text)
	{
		if (m_font && m_text != text)
		{
			m_text = text;
			std::vector<glm::vec4> vertices;
			std::vector<uint16_t> indexes;

			uint16_t lastIndex = 0;
			float offsetX = 0, offsetY = 0;
			for (auto c : text)
			{
				const auto glyphInfo = makeGlyphInfo(m_font, c, offsetX, offsetY);
				offsetX = glyphInfo.offsetX;
				offsetY = glyphInfo.offsetY;

				vertices.emplace_back(glyphInfo.positions[0]);
				vertices.emplace_back(glyphInfo.positions[1]);
				vertices.emplace_back(glyphInfo.positions[2]);
				vertices.emplace_back(glyphInfo.positions[3]);
				indexes.push_back(lastIndex);
				indexes.push_back(lastIndex + 1);
				indexes.push_back(lastIndex + 2);
				indexes.push_back(lastIndex);
				indexes.push_back(lastIndex + 2);
				indexes.push_back(lastIndex + 3);

				lastIndex += 4;
			}
			indexElementCount = indexes.size();

			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indexElementCount, indexes.data(), GL_STATIC_DRAW);
		}
	}

	void Text::Draw(const glm::vec3& position, const glm::vec3& color, const glm::mat4& orthoMat)
	{
		if (m_text.empty() || !m_font || !m_font->texture.IsValid())
			return;

		const glm::mat4 pm = orthoMat * glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));

		cacheShader.Bind();
		cacheShader.SetUniform(m_idAttributeTextColor, { color.x, color.y, color.z });
		cacheShader.SetUniform(m_idAttributeWorldViewProjMatrix, pm);

		m_font->texture.Bind(0);
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glDrawElements(GL_TRIANGLES, indexElementCount, GL_UNSIGNED_SHORT, nullptr);
		VertexArrayBuffer::UnBind();
	}

	bool Text::create(Font* font)
	{
		m_font = font;

		if (!cacheShader.IsValid())
		{
			cacheShader.CreateFromMemories(fontVertexShaderSource, fontFragmentShaderSource);
			cacheShader.Bind();
			m_idAttributeTextColor = cacheShader.GetUniformVariable("textColor");
			m_idAttributeWorldViewProjMatrix = cacheShader.GetUniformVariable("worldViewProjMatrix");
			cacheShader.SetUniform("mainTex", 0);
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vertexBuffer);
		glGenBuffers(1, &indexBuffer);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		return true;
	}

}
#pragma endregion