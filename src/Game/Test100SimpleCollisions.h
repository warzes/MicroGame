#pragma once

Camera ncamera;

void InitTest()
{
	//SetMouseLock(true);
	ncamera.Teleport(0, 0, 0);
	ncamera.Enable();
	ncamera.m_speed = 3;

	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));
}

void CloseTest()
{
}

void FrameTest(float deltaTime)
{
	bool active = IsMouseButtonDown(0);
	SetMouseLock(active);

	const float xpos = GetMouseX();
	const float ypos = GetMouseY();
	static float lastPosX = xpos;
	static float lastPosY = ypos;
	glm::vec2 mouse = tempMath::scale2(glm::vec2((lastPosX - xpos), (lastPosY - ypos)), 200.0f * deltaTime * active);
	lastPosX = xpos;
	lastPosY = ypos;

	glm::vec3 wasdec = tempMath::scale3(glm::vec3(IsKeyboardKeyDown(KEY_A) - IsKeyboardKeyDown(KEY_D), IsKeyboardKeyDown(KEY_E) - IsKeyboardKeyDown(KEY_C), IsKeyboardKeyDown(KEY_W) - IsKeyboardKeyDown(KEY_S)), ncamera.m_speed * deltaTime);

	ncamera.Move(wasdec.x, wasdec.y, wasdec.z);
	ncamera.Fps(mouse.x, mouse.y);

	// animation
	static int paused = 0;
	static float dx = 0, dy = 0;
	if (IsKeyboardKeyDown(KEY_SPACE)) paused ^= 1;
	float delta = deltaTime * !paused;
	dx = dx + delta * 2.0f;
	dy = dy + delta * 0.8f;

	// debug draw
	{
		// grid
		DebugDraw::DrawGrid(0);

		unsigned rgbSel = 0;

		// Triangle-Ray Intersection
		{
			glm::vec3 ro, rd;
			int suc;

			Triangle tri = { glm::vec3(-9,1,28), glm::vec3(-10,0,28), glm::vec3(-11,1,28) };

			// ray
			ro = glm::vec3(-10, -1, 20);
			rd = glm::vec3(-10 + 0.4f * sin(dx), 2.0f * cos(dy), 29.81023f);
			rd = tempMath::sub3(rd, ro);
			rd = tempMath::norm3(rd);

			Ray r = Ray(ro, rd);
			Hit* hit = collide::RayHitTriangle(r, tri);
			if (hit)
			{
				// point of intersection
				DebugDraw::DrawBox(hit->p, glm::vec3(0.10f, 0.10f, 0.10f), RED);

				// intersection normal
				glm::vec3 v = tempMath::add3(hit->p, hit->n);
				DebugDraw::DrawArrow(hit->p, v, BLUE);
			}

			// line
			rd = tempMath::scale3(rd, 10);
			rd = tempMath::add3(ro, rd);
			DebugDraw::DrawLine(ro, rd, RED);

			// triangle
			if (hit) rgbSel = RED;
			else rgbSel = WHITE;

			DebugDraw::DrawTriangle(tri.verts[0], tri.verts[1], tri.verts[2], rgbSel);
		}

		// Plane-Ray Intersection
		{
			glm::vec3 ro, rd;
			glm::mat3 rot;

			// ray
			static float d = 0;
			d += delta * 2.0f;
			ro = glm::vec3(0, -1, 20);
			rd = glm::vec3(0.1f, 0.5f, 9.81023f);
			rd = tempMath::sub3(rd, ro);
			rd = tempMath::norm3(rd);

			// rotation
			tempMath::rotation33(rot, tempMath::deg(d), 0, 1, 0);
			rd = rot * rd;

			// intersection
			Ray r = Ray(ro, rd);
			PlaneOld pl = PlaneOld(glm::vec3(0, 0, 28), glm::vec3(0, 0, 1));
			Hit* hit = collide::RayHitPlane(r, pl);
			if (hit)
			{
				// point of intersection
				DebugDraw::DrawBox(hit->p, glm::vec3(0.10f, 0.10f, 0.10f), RED);

				// intersection normal
				glm::vec3 v = tempMath::add3(hit->p, hit->n);
				DebugDraw::DrawArrow(hit->p, v, BLUE);
			}
			// line
			rd = tempMath::scale3(rd, 9);
			rd = tempMath::add3(ro, rd);
			DebugDraw::DrawLine(ro, rd, RED);

			// plane
			if (hit) rgbSel = RED;
			else rgbSel = WHITE;
			DebugDraw::DrawPlane(glm::vec3(0, 0, 28), glm::vec3(0, 0, 1), 3.0f, rgbSel);
		}

		// Sphere-Ray Intersection
		{
			glm::vec3 ro, rd;
			Sphere s;

			// ray
			ro = glm::vec3(0, -1, 0);
			rd = glm::vec3(0.4f * sin(dx), 2.0f * cos(dy), 9.81023f);
			rd = tempMath::sub3(rd, ro);
			rd = tempMath::norm3(rd);

			Ray r = Ray(ro, rd);
			s = Sphere(glm::vec3(0, 0, 8), 1);
			Hit* hit = collide::RayHitSphere(r, s);
			if (hit)
			{
				// points of intersection
				glm::vec3 in = tempMath::add3(ro, tempMath::scale3(rd, hit->t0));

				DebugDraw::DrawBox(in, glm::vec3(0.05f, 0.05f, 0.05f), GREEN);

				in = tempMath::add3(ro, tempMath::scale3(rd, hit->t1));

				DebugDraw::DrawBox(in, glm::vec3(0.05f, 0.05f, 0.05f), YELLOW);

				// intersection normal
				glm::vec3 v = tempMath::add3(hit->p, hit->n);
				DebugDraw::DrawArrow(hit->p, v, BLUE);
			}
			// line
			rd = tempMath::scale3(rd, 10);
			rd = tempMath::add3(ro, rd);
			DebugDraw::DrawLine(ro, rd, RED);

			// sphere
			if (hit) rgbSel = RED;
			else rgbSel = WHITE;
			DebugDraw::DrawSphere(glm::vec3(0, 0, 8), 1, rgbSel);
		}

		// Ray-AABB
		{
			AABB bounds = AABB(glm::vec3(10 - 0.5f, -0.5f, 7.5f), glm::vec3(10.5f, 0.5f, 8.5f));

			glm::vec3 ro = glm::vec3(10, -1, 0);
			glm::vec3 rd = glm::vec3(10 + 0.4f * sin(dx), 2.0f * cos(dy), 9.81023f);
			rd = tempMath::norm3(tempMath::sub3(rd, ro));
			Ray r = Ray(ro, rd);

			Hit* Hit = collide::RayHitAABB(r, bounds);

			if (Hit)
			{
				// points of intersection
				glm::vec3 in;
				in = tempMath::scale3(rd, Hit->t0);
				in = tempMath::add3(ro, in);

				DebugDraw::DrawBox(in, glm::vec3(0.05f, 0.05f, 0.05f), RED);

				in = tempMath::scale3(rd, Hit->t1);
				in = tempMath::add3(ro, in);

				DebugDraw::DrawBox(in, glm::vec3(0.05f, 0.05f, 0.05f), RED);

				// intersection normal
				glm::vec3 v = tempMath::add3(Hit->p, Hit->n);
				DebugDraw::DrawArrow(Hit->p, v, BLUE);
				rgbSel = RED;
			}
			else rgbSel = WHITE;
			DebugDraw::DrawBox(glm::vec3(10, 0, 8), glm::vec3(1, 1, 1), rgbSel);

			// line
			rd = tempMath::scale3(rd, 10);
			rd = tempMath::add3(ro, rd);
			DebugDraw::DrawLine(ro, rd, RED);
		}

		// Sphere-Sphere intersection*/
		{
			Sphere a = Sphere(glm::vec3(-10, 0, 8), 1);
			Sphere b = Sphere(glm::vec3(-10 + 0.6f * sin(dx), 3.0f * cos(dy), 8), 1);
			Hit* m = collide::SphereHitSphere(a, b);
			if (m)
			{
				glm::vec3 v;
				DebugDraw::DrawBox(m->contact_point, glm::vec3(0.05f, 0.05f, 0.05f), BLUE);
				v = tempMath::add3(m->contact_point, m->normal);
				DebugDraw::DrawArrow(m->contact_point, v, BLUE);
				rgbSel = RED;
			}
			else rgbSel = WHITE;

			DebugDraw::DrawSphere(a.position, 1, rgbSel);
			DebugDraw::DrawSphere(b.position, 1, rgbSel);
		}

		// AABB-AABB intersection*/
		{
			const float x = 10 + 0.6f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = 20.0f;

			AABB a = AABB(glm::vec3(10 - 0.5f, -0.5f, 20 - 0.5f), glm::vec3(10 + 0.5f, 0.5f, 20.5f));
			AABB b = AABB(glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f), glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f));
			Hit* m = collide::AABBHitAABB(a, b);
			if (m)
			{
				glm::vec3 v;
				DebugDraw::DrawBox(m->contact_point, glm::vec3(0.05f, 0.05f, 0.05f), BLUE);
				v = tempMath::add3(m->contact_point, m->normal);
				DebugDraw::DrawArrow(m->contact_point, v, BLUE);
				rgbSel = RED;
			}
			else rgbSel = WHITE;

			DebugDraw::DrawBox(glm::vec3(10, 0, 20), glm::vec3(1, 1, 1), rgbSel);
			DebugDraw::DrawBox(glm::vec3(x, y, z), glm::vec3(1, 1, 1), rgbSel);
		}

		// Capsule-Capsule intersection*/
		{
			const float x = 20 + 0.4f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = 28.5f;

			Capsule a = Capsule(glm::vec3(20.0f, -1.0f, 28.0f), glm::vec3(20.0f, 1.0f, 28.0f), 0.2f);
			Capsule b = Capsule(glm::vec3(x, y - 1.0f, z), glm::vec3(x, y + 1.0f, z - 1.0f), 0.2f);
			Hit* m = collide::CapsuleHitCapsule(a, b);
			if (m)
			{
				glm::vec3 v;
				DebugDraw::DrawBox(m->contact_point, glm::vec3(0.05f, 0.05f, 0.05f), BLUE);
				v = tempMath::add3(m->contact_point, m->normal);
				DebugDraw::DrawArrow(m->contact_point, v, BLUE);
				rgbSel = RED;
			}
			else rgbSel = WHITE;
			DebugDraw::DrawCapsule(glm::vec3(x, y - 1.0f, z), glm::vec3(x, y + 1.0f, z - 1.0f), 0.2f, rgbSel);
			DebugDraw::DrawCapsule(glm::vec3(20.0f, -1.0f, 28.0f), glm::vec3(20.0f, 1.0f, 28.0f), 0.2f, rgbSel);
		}

		// AABB-Sphere intersection*/
		{
			AABB a = AABB(glm::vec3(20 - 0.5f, -0.5f, 7.5f), glm::vec3(20.5f, 0.5f, 8.5f));
			Sphere s = Sphere(glm::vec3(20 + 0.6f * sin(dx), 3.0f * cos(dy), 8), 1);
			Hit* m = collide::AABBHitSphere(a, s);
			if (m)
			{
				glm::vec3 v;
				DebugDraw::DrawBox(m->contact_point, glm::vec3(0.05f, 0.05f, 0.05f), BLUE);
				v = tempMath::add3(m->contact_point, m->normal);
				DebugDraw::DrawArrow(m->contact_point, v, BLUE);
				rgbSel = RED;
			}
			else rgbSel = WHITE;

			DebugDraw::DrawBox(glm::vec3(20, 0, 8), glm::vec3(1, 1, 1), rgbSel);
			DebugDraw::DrawSphere(s.position, 1, rgbSel);
		}

		// Sphere-AABB intersection*/
		{
			const float x = 10 + 0.6f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = -8.0f;

			Sphere s = Sphere(glm::vec3(10, 0, -8), 1);
			AABB a = AABB(glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f), glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f));
			Hit* m = collide::SphereHitAABB(s, a);
			if (m)
			{
				glm::vec3 v;
				DebugDraw::DrawBox(m->contact_point, glm::vec3(0.05f, 0.05f, 0.05f), BLUE);
				v = tempMath::add3(m->contact_point, m->normal);
				DebugDraw::DrawArrow(m->contact_point, v, BLUE);
				rgbSel = RED;
			}
			else rgbSel = WHITE;

			DebugDraw::DrawBox(glm::vec3(x, y, z), glm::vec3(1, 1, 1), rgbSel);
			DebugDraw::DrawSphere(s.position, 1, rgbSel);
		}

		// Capsule-Sphere intersection*/
		{
			Capsule c = Capsule(glm::vec3(-20.5f, -1.0f, 7.5f), glm::vec3(-20 + 0.5f, 1.0f, 8.5f), 0.2f);
			Sphere b = Sphere(glm::vec3(-20 + 0.6f * sin(dx), 3.0f * cos(dy), 8), 1);
			Hit* m = collide::CapsuleHitSphere(c, b);
			if (m)
			{
				glm::vec3 v;
				DebugDraw::DrawBox(m->contact_point, glm::vec3(0.05f, 0.05f, 0.05f), BLUE);
				v = tempMath::add3(m->contact_point, m->normal);
				DebugDraw::DrawArrow(m->contact_point, v, BLUE);
				rgbSel = RED;
			}
			else rgbSel = WHITE;
			DebugDraw::DrawSphere(b.position, 1, rgbSel);
			DebugDraw::DrawCapsule(glm::vec3(-20.5f, -1.0f, 7.5f), glm::vec3(-20 + 0.5f, 1.0f, 8.5f), 0.2f, rgbSel);
		}

		// Sphere-Capsule intersection*/
		{
			const float x = 20 + 0.4f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = -8;

			Sphere s = Sphere(glm::vec3(20, 0, -8), 1);
			Capsule c = Capsule(glm::vec3(x, y - 1.0f, z), glm::vec3(x, y + 1.0f, z - 1.0f), 0.2f);
			Hit* m = collide::SphereHitCapsule(s, c);
			if (m)
			{
				glm::vec3 v;
				DebugDraw::DrawBox(m->contact_point, glm::vec3(0.05f, 0.05f, 0.05f), BLUE);
				v = tempMath::add3(m->contact_point, m->normal);
				DebugDraw::DrawArrow(m->contact_point, v, BLUE);
				rgbSel = RED;
			}
			else rgbSel = WHITE;

			DebugDraw::DrawCapsule(glm::vec3(x, y - 1.0f, z), glm::vec3(x, y + 1.0f, z - 1.0f), 0.2f, rgbSel);
			DebugDraw::DrawSphere(s.position, 1, rgbSel);
		}

		// Capsule-AABB intersection*/
		{
			const float x = -20 + 0.6f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = 28.0f;

			Capsule c = Capsule(glm::vec3(-20.5f, -1.0f, 27.5f), glm::vec3(-20 + 0.5f, 1.0f, 28.5f), 0.2f);
			AABB b = AABB(glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f), glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f));
			Hit* m = collide::CapsuleHitAABB(c, b);
			if (m)
			{
				glm::vec3 v;
				DebugDraw::DrawBox(m->contact_point, glm::vec3(0.05f, 0.05f, 0.05f), BLUE);
				v = tempMath::add3(m->contact_point, m->normal);
				DebugDraw::DrawArrow(m->contact_point, v, BLUE);
				rgbSel = RED;
			}
			else rgbSel = WHITE;
			DebugDraw::DrawBox(glm::vec3(x, y, z), glm::vec3(1, 1, 1), rgbSel);
			DebugDraw::DrawCapsule(glm::vec3(-20.5f, -1.0f, 27.5f), glm::vec3(-20 + 0.5f, 1.0f, 28.5f), 0.2f, rgbSel);
		}

		// AABB-Capsule intersection*/
		{
			const float x = 0.4f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = -8;

			AABB a = AABB(glm::vec3(-0.5f, -0.5f, -8.5f), glm::vec3(0.5f, 0.5f, -7.5f));
			Capsule c = Capsule(glm::vec3(x, y - 1.0f, z), glm::vec3(x, y + 1.0f, z - 1.0f), 0.2f);
			Hit* m = collide::AABBHitCapsule(a, c);
			if (m)
			{
				DebugDraw::DrawBox(m->contact_point, glm::vec3(0.05f, 0.05f, 0.05f), RED);
				DebugDraw::DrawArrow(m->contact_point, tempMath::add3(m->contact_point, m->normal), RED);
				rgbSel = RED;
			}
			else rgbSel = WHITE;

			DebugDraw::DrawCapsule(glm::vec3(x, y - 1.0f, z), glm::vec3(x, y + 1.0f, z - 1.0f), 0.2f, rgbSel);
			DebugDraw::DrawBox(glm::vec3(0, 0, -8.0f), glm::vec3(1, 1, 1), rgbSel);
		}

		// Poly(Pyramid)-Sphere (GJK) intersection*/
		{
			Sphere s = Sphere(glm::vec3(-10 + 0.6f * sin(dx), 3.0f * cos(dy), -8), 1);
			Poly pyr = collide::Pyramid(glm::vec3(-10.5f, -0.5f, -7.5f), glm::vec3(-10.5f, 1.0f, -7.5f), 1.0f);

			collide::GJKResult gjk;
			if (collide::PolyHitSphere(&gjk, pyr, s))
				rgbSel = RED;
			else rgbSel = WHITE;

			DebugDraw::DrawSphere(s.position, 1, rgbSel);
			DebugDraw::DrawPyramid(glm::vec3(-10.5f, -0.5f, -7.5f), 0.5f/*glm::vec3(-10.5f,1.0f,-7.5f)*/, 1.0f, rgbSel);

			DebugDraw::DrawBox(gjk.p0, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawBox(gjk.p1, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawLine(gjk.p0, gjk.p1, rgbSel);
		}

		// Poly(Diamond)-Sphere (GJK) intersection*/
		{
			Sphere s = Sphere(glm::vec3(-20 + 0.6f * sin(dx), 3.0f * cos(dy), -8), 1);
			Poly dmd = collide::Diamond(glm::vec3(-20.5f, -0.5f, -7.5f), glm::vec3(-20.5f, 1.0f, -7.5f), 0.5f);

			collide::GJKResult gjk;
			if (collide::PolyHitSphere(&gjk, dmd, s))
				rgbSel = RED;
			else rgbSel = WHITE;

			DebugDraw::DrawSphere(s.position, 1, rgbSel);
			DebugDraw::DrawDiamond(glm::vec3(-20.5f, -0.5f, -7.5f), glm::vec3(-20.5f, 1.0f, -7.5f), 0.5f, rgbSel);

			DebugDraw::DrawBox(gjk.p0, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawBox(gjk.p1, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawLine(gjk.p0, gjk.p1, rgbSel);
		}
		// Poly(Pyramid)-Capsule (GJK) intersection*/
		{
			const float x = 0.4f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = -15;

			Capsule c = Capsule(glm::vec3(x, y - 1.0f, z), glm::vec3(x, y + 1.0f, z), 0.2f);
			Poly pyr = collide::Pyramid(glm::vec3(-0.5f, -0.5f, -15.5f), glm::vec3(-0.5f, 1.0f, -15.5f), 1.0f);

			collide::GJKResult gjk;
			if (collide::PolyHitCapsule(&gjk, pyr, c))
				rgbSel = RED;
			else rgbSel = WHITE;

			DebugDraw::DrawCapsule(c.a, c.b, c.r, rgbSel);
			DebugDraw::DrawPyramid(glm::vec3(-0.5f, -0.5f, -15.5f), 0.5f/*glm::vec3(-0.5f,1.0f,-15.5f)*/, 1.0f, rgbSel);

			DebugDraw::DrawBox(gjk.p0, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawBox(gjk.p1, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawLine(gjk.p0, gjk.p1, rgbSel);
		}

		// Poly(Diamond)-Capsule (GJK) intersection*/
		{
			const float x = -10 + 0.4f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = -15;

			Capsule c = Capsule(glm::vec3(x, y - 1.0f, z), glm::vec3(x, y + 1.0f, z), 0.2f);
			Poly dmd = collide::Diamond(glm::vec3(-10.5f, -0.5f, -15.5f), glm::vec3(-10.5f, 1.0f, -15.5f), 0.5f);

			collide::GJKResult gjk;
			if (collide::PolyHitCapsule(&gjk, dmd, c))
				rgbSel = RED;
			else rgbSel = WHITE;

			DebugDraw::DrawCapsule(c.a, c.b, c.r, rgbSel);
			DebugDraw::DrawDiamond(glm::vec3(-10.5f, -0.5f, -15.5f), glm::vec3(-10.5f, 1.0f, -15.5f), 0.5f, rgbSel);

			DebugDraw::DrawBox(gjk.p0, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawBox(gjk.p1, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawLine(gjk.p0, gjk.p1, rgbSel);
		}

		// Poly(Diamond)-Poly(Pyramid) (GJK) intersection*/
		{
			const float x = -20 + 0.4f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = -15;

			Poly pyr = collide::Pyramid(glm::vec3(x, y - 0.5f, z), glm::vec3(x, y + 1, z), 0.8f);
			Poly dmd = collide::Diamond(glm::vec3(-20.5f, -0.5f, -15.5f), glm::vec3(-20.5f, 1.0f, -15.5f), 0.5f);

			collide::GJKResult gjk;
			if (collide::PolyHitPoly(&gjk, dmd, pyr))
				rgbSel = RED;
			else rgbSel = WHITE;

			DebugDraw::DrawPyramid(glm::vec3(x, y - 0.5f, z), 1/*glm::vec3(x,y+1,z)*/, 1/*0.8f*/, rgbSel);
			DebugDraw::DrawDiamond(glm::vec3(-20.5f, -0.5f, -15.5f), glm::vec3(-20.5f, 1.0f, -15.5f), 0.5f, rgbSel);

			DebugDraw::DrawBox(gjk.p0, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawBox(gjk.p1, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawLine(gjk.p0, gjk.p1, rgbSel);
		}

		// Poly(Pyramid)-Poly(Diamond) (GJK) intersection*/
		{
			const float x = 10 + 0.4f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = -15;

			Poly dmd = collide::Diamond(glm::vec3(x, y - 0.5f, z), glm::vec3(x, y + 1, z), 0.5f);
			Poly pyr = collide::Pyramid(glm::vec3(10.5f, -0.5f, -15.5f), glm::vec3(10.5f, 1.0f, -15.5f), 1.0f);

			collide::GJKResult gjk;
			if (collide::PolyHitPoly(&gjk, dmd, pyr))
				rgbSel = RED;
			else rgbSel = WHITE;

			DebugDraw::DrawDiamond(glm::vec3(x, y - 0.5f, z), glm::vec3(x, y + 1, z), 0.5f, rgbSel);
			DebugDraw::DrawPyramid(glm::vec3(10.5f, -0.5f, -15.5f), 0.5f/*glm::vec3(10.5f,1.0f,-15.5f)*/, 1.0f, rgbSel);

			DebugDraw::DrawBox(gjk.p0, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawBox(gjk.p1, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawLine(gjk.p0, gjk.p1, rgbSel);
		}

		// Poly(Diamond)-AABB (GJK) intersection*/
		{
			const float x = 20 + 0.4f * sin(dx);
			const float y = 3.0f * cos(dy);
			const float z = -15;

			Poly dmd = collide::Diamond(glm::vec3(x, y - 0.5f, z), glm::vec3(x, y + 1, z), 0.5f);
			AABB a = AABB(glm::vec3(19.5f, -0.5f, -14.5f), glm::vec3(20.5f, 0.5f, -15.5f));

			collide::GJKResult gjk;
			if (collide::PolyHitAABB(&gjk, dmd, a))
				rgbSel = RED;
			else rgbSel = WHITE;

			DebugDraw::DrawDiamond(glm::vec3(x, y - 0.5f, z), glm::vec3(x, y + 1, z), 0.5f, rgbSel);
			DebugDraw::DrawBox(glm::vec3(20, 0, -15), glm::vec3(1, 1, 1), rgbSel);

			DebugDraw::DrawBox(gjk.p0, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawBox(gjk.p1, glm::vec3(0.05f, 0.05f, 0.05f), rgbSel);
			DebugDraw::DrawLine(gjk.p0, gjk.p1, rgbSel);
		}

		DebugDraw::Flush(ncamera);
	}
}