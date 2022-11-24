#pragma once

constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uColor;

out vec2 vTexCoord;
out vec3 vColor;

void main()
{
	gl_Position = uProjection * uView * uWorld * vec4(vPos, 1.0);
	vTexCoord = aTexCoord;
	vColor = uColor;
}
)";
constexpr const char* fragment_shader_text = R"(
#version 330 core

in vec2 vTexCoord;
in vec3 vColor;

uniform sampler2D uSampler;

out vec4 fragColor;

void main()
{
	vec4 textureClr = texture(uSampler, vTexCoord);
	if (textureClr.a < 0.02) discard;
	fragColor = textureClr;
}
)";

ShaderProgram shader;
UniformLocation worldUniform;
UniformLocation viewUniform;
UniformLocation projectionUniform;
UniformLocation colorUniform;

g3d::Model model;
Poly modelPoly;
g3d::Material material;
Transform transform;

Camera ncamera;

namespace Player
{
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 speed = glm::vec3(0.0f);
	glm::vec3 lastSpeed = glm::vec3(0.0f);
	glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
	float radius = 0.2f;
	bool onGround = false;
	float stepDownSize = 0.075;
}



// finds the closest point on the triangle from the source point given
// sources: https://wickedengine.net/2020/04/26/capsule-collision-detection/
float TrianglePoint(
	float tri_0_x, float tri_0_y, float tri_0_z,
	float tri_1_x, float tri_1_y, float tri_1_z,
	float tri_2_x, float tri_2_y, float tri_2_z,
	float tri_n_x, float tri_n_y, float tri_n_z,
	float src_x, float src_y, float src_z,
	// out
	glm::vec3& outItx,
	glm::vec3& outNormal)
{
	// recalculate surface normal of this triangle
	glm::vec3 side1 = { tri_1_x - tri_0_x, tri_1_y - tri_0_y, tri_1_z - tri_0_z };
	glm::vec3 side2 = { tri_1_x - tri_0_x, tri_1_y - tri_0_y, tri_1_z - tri_0_z };
	glm::vec3 norm = glm::normalize(glm::cross(side1, side2));

	// distance from src to a vertex on the triangle
	float dist = glm::dot({ src_x - tri_0_x, src_y - tri_0_y, src_z - tri_0_z }, norm);

	// itx stands for intersection
	glm::vec3 itx = { src_x - norm.x * dist, src_y - norm.y * dist, src_z - norm.z * dist };

	// determine whether itx is inside the triangle
	// project it onto the triangleand return if this is the case
	glm::vec3 c0 = glm::cross(glm::vec3(itx.x - tri_0_x, itx.y - tri_0_y, itx.z - tri_0_z), glm::vec3(tri_1_x - tri_0_x, tri_1_y - tri_0_y, tri_1_z - tri_0_z));
	glm::vec3 c1 = glm::cross(glm::vec3(itx.x - tri_1_x, itx.y - tri_1_y, itx.z - tri_1_z), glm::vec3(tri_2_x - tri_1_x, tri_2_y - tri_1_y, tri_2_z - tri_1_z));
	glm::vec3 c2 = glm::cross(glm::vec3(itx.x - tri_2_x, itx.y - tri_2_y, itx.z - tri_2_z), glm::vec3(tri_0_x - tri_2_x, tri_0_y - tri_2_y, tri_0_z - tri_2_z));

	if (glm::dot(c0, norm) <= 0.0f &&
		glm::dot(c1, norm) <= 0.0f &&
		glm::dot(c2, norm) <= 0.0f)
	{
		norm = { src_x - itx.x, src_y - itx.y, src_z - itx.z };
		
		// the sphere is inside the triangle, so the normal is zero
		// instead, just return the triangle's normal
		if (norm.x == 0 && norm.y == 0 && norm.z == 0)
			outNormal = { tri_n_x, tri_n_y, tri_n_z };
		else
			outNormal = norm;

		outItx = itx;
		return sqrt(norm.x * norm.x + norm.y * norm.y + norm.z * norm.z);
	}

	// itx is outside triangle
	// find points on all three line segments that are closest to itx
	// if distance between itxand one of these three closest points is in range, there is an intersection
	glm::vec3 line1 = Collisions::ClosestPointOnLineSegment({ tri_0_x, tri_0_y, tri_0_z }, { tri_1_x, tri_1_y, tri_1_z }, { src_x, src_y, src_z });
	dist = (src_x - line1.x) * (src_x - line1.x) + (src_y - line1.y) * (src_y - line1.y) + (src_z - line1.z) * (src_z - line1.z);
	float smallestDist = dist;
	itx = line1;

	glm::vec3 line2 = Collisions::ClosestPointOnLineSegment({ tri_1_x, tri_1_y, tri_1_z }, { tri_2_x, tri_2_y, tri_2_z }, { src_x, src_y, src_z });
	dist = (src_x - line2.x) * (src_x - line2.x) + (src_y - line2.y) * (src_y - line2.y) + (src_z - line2.z) * (src_z - line2.z);
	if (dist < smallestDist)
	{
		smallestDist = dist;
		itx = line2;
	}

	glm::vec3 line3 = Collisions::ClosestPointOnLineSegment({ tri_2_x, tri_2_y, tri_2_z }, { tri_0_x, tri_0_y, tri_0_z }, { src_x, src_y, src_z });
	dist = (src_x - line3.x) * (src_x - line3.x) + (src_y - line3.y) * (src_y - line3.y) + (src_z - line3.z) * (src_z - line3.z);
	if (dist < smallestDist)
	{
		smallestDist = dist;
		itx = line3;
	}

	norm = { src_x - itx.x, src_y - itx.y, src_z - itx.z };

	// the sphere is inside the triangle, so the normal is zero
	// instead, just return the triangle's normal
	if (norm.x == 0 && norm.y == 0 && norm.z == 0)
		outNormal = { tri_n_x, tri_n_y, tri_n_z };
	else
		outNormal = norm;

	outItx = itx;
	return sqrt(norm.x * norm.x + norm.y * norm.y + norm.z * norm.z);
}

// detects a collision between a triangleand a sphere
// sources: https://wickedengine.net/2020/04/26/capsule-collision-detection/
float TriangleSphere(
	float tri_0_x, float tri_0_y, float tri_0_z,
	float tri_1_x, float tri_1_y, float tri_1_z,
	float tri_2_x, float tri_2_y, float tri_2_z,
	float tri_n_x, float tri_n_y, float tri_n_z,
	float src_x, float src_y, float src_z, 
	float radius,
	// out
	glm::vec3& outItx,
	glm::vec3& outNormal
)
{
	// recalculate surface normal of this triangle
	glm::vec3 side1 = { tri_1_x - tri_0_x, tri_1_y - tri_0_y, tri_1_z - tri_0_z };
	glm::vec3 side2 = { tri_2_x - tri_0_x, tri_2_y - tri_0_y, tri_2_z - tri_0_z };
	glm::vec3 norm = glm::normalize(glm::cross(side1, side2));

	// distance from src to a vertex on the triangle
	float dist = glm::dot({ src_x - tri_0_x, src_y - tri_0_y, src_z - tri_0_z }, norm);

	// collision not possible, just return
	if (dist < -radius || dist > radius)
		return  std::numeric_limits<float>::infinity();// ??? что вернуть

	// itx stands for intersection
	glm::vec3 itx = { src_x - norm.x * dist, src_y - norm.y * dist, src_z - norm.z * dist };

	// determine whether itx is inside the triangle
	// project it onto the triangle and return if this is the case
	glm::vec3 c0 = glm::cross(
		glm::vec3{ itx.x - tri_0_x, itx.y - tri_0_y, itx.z - tri_0_z }, 
		glm::vec3{ tri_1_x - tri_0_x, tri_1_y - tri_0_y, tri_1_z - tri_0_z });
	glm::vec3 c1 = glm::cross(
		glm::vec3{ itx.x - tri_1_x, itx.y - tri_1_y, itx.z - tri_1_z },
		glm::vec3{ tri_2_x - tri_1_x, tri_2_y - tri_1_y, tri_2_z - tri_1_z });
	glm::vec3 c2 = glm::cross(
		glm::vec3{ itx.x - tri_2_x, itx.y - tri_2_y, itx.z - tri_2_z },
		glm::vec3{ tri_0_x - tri_2_x, tri_0_y - tri_2_y, tri_0_z - tri_2_z });

	if (glm::dot(c0, norm) <= 0.0f &&
		glm::dot(c1, norm) <= 0.0f &&
		glm::dot(c2, norm) <= 0.0f)
	{
		norm = { src_x - itx.x, src_y - itx.y, src_z - itx.z };

		// the sphere is inside the triangle, so the normal is zero instead, just return the triangle's normal
		if (norm.x == 0 && norm.y == 0 && norm.z == 0)
			outNormal = { tri_n_x, tri_n_y, tri_n_z };
		else
			outNormal = norm;

		outItx = itx;
		return sqrt(norm.x * norm.x + norm.y * norm.y + norm.z * norm.z);
	}

	// itx is outside triangle
	// find points on all three line segments that are closest to itx
	// if distance between itxand one of these three closest points is in range, there is an intersection
	float radiussq = radius * radius;
	float smallestDist = std::numeric_limits<float>::infinity();

	glm::vec3 line1 = Collisions::ClosestPointOnLineSegment({ tri_0_x, tri_0_y, tri_0_z }, { tri_1_x, tri_1_y, tri_1_z }, { src_x, src_y, src_z });
	dist = (src_x - line1.x) * (src_x - line1.x) + (src_y - line1.y) * (src_y - line1.y) + (src_z - line1.z) * (src_z - line1.z);
	if (dist <= radiussq)
	{
		smallestDist = dist;
		itx = line1;
	}

	glm::vec3 line2 = Collisions::ClosestPointOnLineSegment({ tri_1_x, tri_1_y, tri_1_z }, { tri_2_x, tri_2_y, tri_2_z }, { src_x, src_y, src_z });
	dist = (src_x - line2.x) * (src_x - line2.x) + (src_y - line2.y) * (src_y - line2.y) + (src_z - line2.z) * (src_z - line2.z);
	if (dist < smallestDist)
	{
		smallestDist = dist;
		itx = line2;
	}

	glm::vec3 line3 = Collisions::ClosestPointOnLineSegment({ tri_2_x, tri_2_y, tri_2_z }, { tri_0_x, tri_0_y, tri_0_z }, { src_x, src_y, src_z });
	dist = (src_x - line3.x) * (src_x - line3.x) + (src_y - line3.y) * (src_y - line3.y) + (src_z - line3.z) * (src_z - line3.z);
	if (dist < smallestDist)
	{
		smallestDist = dist;
		itx = line3;
	}

	norm = { src_x - itx.x, src_y - itx.y, src_z - itx.z };

	// the sphere is inside the triangle, so the normal is zero instead, just return the triangle's normal
	if (norm.x == 0 && norm.y == 0 && norm.z == 0)
		outNormal = { tri_n_x, tri_n_y, tri_n_z };
	else
		outNormal = norm;

	outItx = itx;
	return sqrt(norm.x * norm.x + norm.y * norm.y + norm.z * norm.z);
}

// finds the collision point between a triangle and a capsule
// capsules are defined with two pointsand a radius
// sources: https://wickedengine.net/2020/04/26/capsule-collision-detection/
float TriangleCapsule(
	float tri_0_x, float tri_0_y, float tri_0_z,
	float tri_1_x, float tri_1_y, float tri_1_z,
	float tri_2_x, float tri_2_y, float tri_2_z,
	float n_x, float n_y, float n_z,
	float tip_x, float tip_y, float tip_z,
	float base_x, float base_y, float base_z,
	float a_x, float a_y, float a_z,
	float b_x, float b_y, float b_z,
	float capn_x, float capn_y, float capn_z,
	float radius,
	// out
	glm::vec3& outItx,
	glm::vec3& outNormal
)
{
	// find the normal of this triangle
	// tbd if necessary, this sometimes fixes weird edgecases
	glm::vec3 side1 = { tri_1_x - tri_0_x, tri_1_y - tri_0_y, tri_1_z - tri_0_z };
	glm::vec3 side2 = { tri_2_x - tri_0_x, tri_2_y - tri_0_y, tri_2_z - tri_0_z };
	glm::vec3 norm = glm::normalize(glm::cross(side1, side2));

	float dotOfNormals = abs(glm::dot(norm, { capn_x, capn_y, capn_z }));

	// default reference point to an arbitrary point on the triangle
	// for when dotOfNormals is 0, because then the capsule is parallel to the triangle
	glm::vec3 ref = { tri_0_x, tri_0_y, tri_0_z }; // почему тут реф? может это ссылка которая должна изменить tri_0?

	if (dotOfNormals > 0)
	{
		// capsule is not parallel to the triangle's plane
		// find where the capsule's normal vector intersects the triangle's plane

		float t = glm::dot(norm, glm::vec3((tri_0_x - base_x) / dotOfNormals, (tri_0_y - base_y) / dotOfNormals, (tri_0_z - base_z) / dotOfNormals));

		glm::vec3 plane_itx = { base_x + capn_x * t, base_y + capn_y * t, base_z + capn_z * t };
		glm::vec3 empty = glm::vec3{ 0.0 };

		// then clamp that plane intersect point onto the triangle itself this is the new reference point
		/*float unused = */TrianglePoint(
			// in
			tri_0_x, tri_0_y, tri_0_z,
			tri_1_x, tri_1_y, tri_1_z,
			tri_2_x, tri_2_y, tri_2_z,
			n_x, n_y, n_z,
			plane_itx.x, plane_itx.y, plane_itx.z,
			//out
			ref, empty
		);
	}

	// find the closest point on the capsule line to the reference point
	glm::vec3 c = Collisions::ClosestPointOnLineSegment({ a_x, a_y, a_z }, { b_x, b_y, b_z }, ref);

	// do a sphere cast from that closest point to the triangle and return the result
	return TriangleSphere(
		tri_0_x, tri_0_y, tri_0_z,
		tri_1_x, tri_1_y, tri_1_z,
		tri_2_x, tri_2_y, tri_2_z,
		n_x, n_y, n_z,
		c.x, c.y, c.z, radius,
		outItx, outNormal
	);
}


float TriangleCapsuleFindClosest(const Poly& poly,
	float tip_x, float tip_y, float tip_z,
	float base_x, float base_y, float base_z,
	float a_x, float a_y, float a_z,
	float b_x, float b_y, float b_z,
	float norm_x, float norm_y, float norm_z,
	float radius,
	//out
	glm::vec3& outWhere,
	glm::vec3& outNorm)
{
	//declare the variables that will be returned by the function
	float finalLength = std::numeric_limits<float>::infinity(), where_x, where_y, where_z, n_x, n_y, n_z;
	// cache references to this model's properties for efficiency
	float translation_x = 0.0f, translation_y = 0.0f, translation_z = 0.0f, scale_x =1.0f, scale_y = 1.0f, scale_z = 1.0f;

	// TODO: translation_n и scale_т - от модели

	// пробую свои нормали
	std::vector<glm::vec3> vertNormal;
	vertNormal.resize(poly.verts.size());

	for (int i = 0; i < poly.verts.size(); i+=3)
	{
		// apply the function given with the arguments given also supply the points of the current triangle
		// TODO: хранить нормаль в меше
		//glm::vec3 normalTriangle = glm::normalize(
		//	verts[v][6] * scale_x,
		//	verts[v][7] * scale_x,
		//	verts[v][8] * scale_x
		//);
		//glm::vec3 normalTriangle = glm::normalize(glm::triangleNormal(
		//	poly.verts[i + 0] * scale_x,
		//	poly.verts[i + 1] * scale_y,
		//	poly.verts[i + 2] * scale_z));
		glm::vec3 normalTriangle = glm::vec3(0.0f);

		glm::vec3 w;
		glm::vec3 n;

		float length = TriangleCapsule(
			poly.verts[i + 0].x * scale_x + translation_x,
			poly.verts[i + 0].y * scale_y + translation_y,
			poly.verts[i + 0].z * scale_z + translation_z,
			poly.verts[i + 1].x * scale_x + translation_x,
			poly.verts[i + 1].y * scale_y + translation_y,
			poly.verts[i + 1].z * scale_z + translation_z,
			poly.verts[i + 2].x * scale_x + translation_x,
			poly.verts[i + 2].y * scale_y + translation_y,
			poly.verts[i + 2].z * scale_z + translation_z,
			normalTriangle.x,
			normalTriangle.y,
			normalTriangle.z,
			tip_x, tip_y, tip_z,
			base_x, base_y, base_z,
			a_x, a_y, a_z,
			b_x, b_y, b_z,
			norm_x, norm_y, norm_z,
			radius, 
			w, n
		);

		// if something was hit and either the finalLength is not yet defined or the new length is closer then update the collision information
		if (length < finalLength)
		{
			finalLength = length;
			where_x = w.x;
			where_y = w.y;
			where_z = w.z;
			n_x = n.x;
			n_y = n.y;
			n_z = n.z;
		}
	}

	// normalize the normal vector before it is returned
	glm::vec3 norm = glm::normalize(glm::vec3{ n_x, n_y, n_z });

	// return all the information in a standardized way
	outWhere = glm::vec3{ where_x, where_y, where_z };
	outNorm = norm;
	return finalLength;
}

float CapsuleIntersection(const Poly& poly, 
	float tip_x, float tip_y, float tip_z,
	float base_x, float base_y, float base_z,
	float radius,
	//out
	glm::vec3& outWhere,
	glm::vec3& outNorm)
{
	// the normal vector coming out the tip of the capsule
	glm::vec3 norm = glm::normalize(glm::vec3{ tip_x - base_x, tip_y - base_y, tip_z - base_z });

	// the baseand tip, inset by the radius
	// these two coordinates are the actual extent of the capsule sphere line
	glm::vec3 a = { base_x + norm.x * radius, base_y + norm.y * radius, base_z + norm.z * radius };
	glm::vec3 b = { tip_x - norm.x * radius, tip_y - norm.y * radius, tip_z - norm.z * radius };

	return TriangleCapsuleFindClosest(poly,
		tip_x, tip_y, tip_z,
		base_x, base_y, base_z,
		a.x, a.y, a.z,
		b.x, b.y, b.z, 
		norm.x, norm.y, norm.z,
		radius,
		outWhere,
		outNorm);
}

// collide against all models in my collision list and return the collision against the closest one
float collisionTest(glm::vec3& pos, glm::vec3& normal, float mx, float my, float mz)
{
	float bestLength = 0.0f;
	float bx = 0.0f;
	float by = 0.0f;
	float bz = 0.0f;
	float bnx = 0.0f;
	float bny = 0.0f;
	float bnz = 0.0f;

	// перебор доступных моделей
	// while(models)
	{
		glm::vec3 retPos;
		glm::vec3 retNorm;
		float len = CapsuleIntersection(modelPoly, 
			// capsule
			Player::position.x + mx,
			Player::position.y + my - 0.15f,
			Player::position.z + mz,
			Player::position.x + mx,
			Player::position.y + my + 0.5f,
			Player::position.z + mz,
			0.2f,
			// out
			retPos, retNorm
		);


		if (/*len > 0.0f &&*/ bestLength == 0.0f || len < bestLength)
		{
			bestLength = len;
			bx = retPos.x;
			by = retPos.y;
			bz = retPos.z;
			bnx = retNorm.x;
			bny = retNorm.y;
			bnz = retNorm.z;
		}
	}

	pos.x = bx;
	pos.y = by;
	pos.z = bz;
	normal.x = bnx;
	normal.y = bny;
	normal.z = bnz;
	return bestLength;
}

void moveAndSlide(glm::vec3& outNormal, glm::vec3& outPos, float mx, float my, float mz)
{
	float len = 0.0;
	glm::vec3 normal = glm::vec3(0.0f);
	glm::vec3 tempPos = glm::vec3(0.0f);
	len = collisionTest(tempPos, normal, mx, my, mz);

	Player::position.x = Player::position.x + mx;
	Player::position.y = Player::position.y + my;
	Player::position.z = Player::position.z + mz;

	bool ignoreSlopes = normal.y < -0.7;
	//if (len > 0.0f)
	{
		float speedLength = sqrt(mx * mx + my * my + mz * mz);
		if (speedLength > 0)
		{
			float xNorm = mx / speedLength;
			float yNorm = my / speedLength;
			float zNorm = mz / speedLength;

			float dot = xNorm * normal.x + yNorm * normal.y + zNorm * normal.z;

			float xPush = normal.x * dot;
			float yPush = normal.y * dot;
			float zPush = normal.z * dot;

			//modify output vector based on normal
			my = (yNorm - yPush) * speedLength;
			if (ignoreSlopes) my = 0.0f;

			if (!ignoreSlopes)
			{
				mx = (xNorm - xPush) * speedLength;
				mz = (zNorm - zPush) * speedLength;
			}
		}

		// rejections
		Player::position.y = Player::position.y - normal.y * (len - Player::radius);
		if (!ignoreSlopes)
		{
			Player::position.x = Player::position.x - normal.x * (len - Player::radius);
			Player::position.z = Player::position.z - normal.z * (len - Player::radius);
		}
	}

	outNormal = normal;
	outPos = { mx, my, mz };
}

void PlayerUpdate(float deltaTime)
{
	// collect inputs
	int moveX = 0;
	int moveY = 0;
	const float speed = 0.015f;
	const float friction = 0.75f;
	const float gravity = 0.005f;
	const float jump = 1.0f / 12.0f;
	const float maxFallSpeed = 0.25;
	glm::vec3 normal = glm::vec3(0.0f);
	glm::vec3 newPos = glm::vec3(0.0f);
	float posEmpty = 0.0f;

	// friction
	Player::speed.x = Player::speed.x * friction;
	Player::speed.z = Player::speed.z * friction;

	// gravity
	Player::speed.y = Min(Player::speed.y + gravity, maxFallSpeed);

	// input move
	if (IsKeyboardKeyDown(KEY_UP))
		moveY--;
	if (IsKeyboardKeyDown(KEY_LEFT))
		moveX--;
	if (IsKeyboardKeyDown(KEY_DOWN))
		moveY++;
	if (IsKeyboardKeyDown(KEY_RIGHT))
		moveX++;
	if (IsKeyboardKeyDown(KEY_SPACE) && Player::onGround)
		Player::speed.y = Player::speed.y - jump;

	// do some trigonometry on the inputs to make movement relative to camera's direction also to make the player not move faster in diagonal directions
	if (moveX != 0 || moveY != 0)
	{
		float angle = atan2(moveY, moveX);
		// TODO:
		float direction = 0.0f;//g3d.camera.getDirectionPitch();
		float directionX = cos(direction + angle) * speed;
		float directionZ = sin(direction + angle + Pi) * speed;

		Player::speed.x = Player::speed.x + directionX;
		Player::speed.z = Player::speed.z + directionZ;
	}

	// vertical movement and collision check
	moveAndSlide(normal, newPos, posEmpty, Player::speed.y, posEmpty);
	Player::speed.y = newPos.y;

	// ground check
	bool wasOnGround = Player::onGround;
	Player::onGround = normal.y < -0.7;

	// smoothly walk down slopes
	if (!Player::onGround && wasOnGround && Player::speed.y > 0)
	{
		// TODO:
		float len = 1.0;
		glm::vec3 tempPos = glm::vec3(0.0f);
		normal = glm::vec3(0.0f);
		len = collisionTest(tempPos, normal, posEmpty, Player::stepDownSize, posEmpty);
		float mx = 0.0f;
		float my = Player::stepDownSize;
		float mz = 0.0f;
		
		//if (len > 0.0f)
		{
			// do the position change only if a collision was actually detected
			Player::position.y = Player::position.y + my;

			float speedLength = sqrt(mx * mx + my * my + mz * mz);
			if (speedLength > 0)
			{
				float xNorm = mx / speedLength;
				float yNorm = my / speedLength;
				float zNorm = mz / speedLength;

				float dot = xNorm * normal.x + yNorm * normal.y + zNorm * normal.z;
				float xPush = normal.x * dot;
				float yPush = normal.y * dot;
				float zPush = normal.z * dot;

				//modify output vector based on normal
				my = (yNorm - yPush) * speedLength;
			}

			// rejections
			Player::position.y = Player::position.y - normal.y * (len - Player::radius);
			Player::speed.y = 0;
			Player::onGround = true;
		}
	}

	// wall movement and collision check
	moveAndSlide(normal, newPos, Player::speed.x, posEmpty, Player::speed.z);
	Player::speed.x = newPos.x;
	Player::speed.z = newPos.z;
	Player::lastSpeed = Player::speed;
}

void InitTest()
{
	// Init Camera
	{
		ncamera.Teleport(0, 3, -6);
		ncamera.LookAt(glm::vec3(0, 0, 0));
		ncamera.Enable();
		ncamera.m_speed = 3;
	}

	// Load shader
	{
		shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
		shader.Bind();
		shader.SetUniform("uSampler", 0);
		worldUniform = shader.GetUniformVariable("uWorld");
		viewUniform = shader.GetUniformVariable("uView");
		projectionUniform = shader.GetUniformVariable("uProjection");
		colorUniform = shader.GetUniformVariable("uColor");
	}

	// Load Texture
	{
		material.diffuseTexture = TextureLoader::LoadTexture2D("../data/textures/tileset.png");
	}

	// Load geometry
	{
		model.Create("../data/models/map.obj");
		model.SetMaterial(material);
		transform.Translate(0, 0, 0);

		modelPoly = model.GetPoly();
	}

	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));
}

void CloseTest()
{
	shader.Destroy();
}

void FrameTest(float deltaTime)
{
	// camera
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
	}

	PlayerUpdate(deltaTime);
	float fraction = 1.0f; // accumulator/frametime
	glm::vec3 playerPos = {
		Player::position.x + Player::speed.x * fraction,
		Player::position.y,
		Player::position.z + Player::speed.z * fraction,
	};

	

	shader.Bind();

	shader.SetUniform(viewUniform, ncamera.m_view);
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());
	shader.SetUniform(colorUniform, glm::vec3(0, 0, 1));
	shader.SetUniform(worldUniform, transform.GetWorld());
	model.Draw();	

	//DebugDraw::DrawGrid(0);
	DebugDraw::DrawCapsule(
		glm::vec3(playerPos.x, playerPos.y, playerPos.z),
		glm::vec3(playerPos.x, playerPos.y + 1, playerPos.z),
		0.2f, RED);
	DebugDraw::Flush(ncamera);

	std::string ss = std::to_string(playerPos.x) + "|" + std::to_string(playerPos.y) + "|" + std::to_string(playerPos.z);
	puts(ss.c_str());
}