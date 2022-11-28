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
in float posY;

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
g3d::Material material;
Transform transform;

Camera ncamera;

// NEW

/* collision
  Handles the collision checking between
  an ellipsoid and a triangle.

  Based on Kasper Fauerbys's paper.
  http://www.peroxide.dk/papers/collision/collision.pdf
*/
/* entity
  The 3d entity component of the engine,
  defined as an ellipsoid that slides around
  the game level.
  Part of the response step (as well as detection step)
  is based on the following paper by Kasper Fauerby.
  http://www.peroxide.dk/papers/collision/collision.pdf
  A more robust response step has been implemented
  based on the follow up paper to the above by
  Jeff Linahan.
  https://arxiv.org/ftp/arxiv/papers/1211/1211.0059.pdf
*/

#define unitsPerMeter 100.0f

#define SLIDE_BIAS 0.008
#define VERY_CLOSE_DIST 0.001
#define SLOPE_WALK_ANGLE 0.80

class Plane3 
{
public:
	glm::vec4 equation;
	glm::vec3 origin;
	glm::vec3 normal;
	Plane3() = default;
	Plane3(const glm::vec3& origin, const glm::vec3& normal);
	Plane3(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);

	bool IsFrontFacingTo(const glm::vec3& direction) const;
	float SignedDistanceTo(const glm::vec3& point) const;
};

Plane3::Plane3(const glm::vec3& origin, const glm::vec3& normal)
{
	this->origin = origin;
	this->normal = normal;
	equation.x = normal.x;
	equation.y = normal.y;
	equation.z = normal.z;
	equation.w = -glm::dot(origin, normal);
}

// Construct from triangle:
Plane3::Plane3(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
	normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));

	origin = p1;

	equation.x = normal.x;
	equation.y = normal.y;
	equation.z = normal.z;
	equation.w = -glm::dot(normal, origin);
}

float Plane3::SignedDistanceTo(const glm::vec3& point) const
{
	return (glm::dot(point, normal)) + equation.w;
}

bool Plane3::IsFrontFacingTo(const glm::vec3& direction) const
{
	const float d = glm::dot(normal, direction);
	return (d <= 0.0f);
}

class CollisionPacket
{
public:
	// Information about the move being requested: (in R3)
	glm::vec3 R3Velocity;
	glm::vec3 R3Position;

	// ellipsoid space
	glm::vec3 eRadius; // ellipsoid radius
	// Information about the move being requested: (in eSpace)
	glm::vec3 velocity;
	glm::vec3 normalizedVelocity;
	glm::vec3 basePoint;

	// original tri points
	glm::vec3 a, b, c;

	// Hit information
	bool foundCollision;
	float nearestDistance;
	float t;
	glm::vec3 intersectionPoint;

	Plane3 plane;

	// iteration depth
	int collisionRecursionDepth;
};

// Assumes: p1,p2 and p3 are given in ellisoid space:
inline void CheckCollisionsTriangle(CollisionPacket* colPackage, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
	// Make the Plane containing this triangle.
	Plane3 trianglePlane(p1, p2, p3);
	// Is triangle front-facing to the velocity vector?
	// only check front-facing triangles
	if (!trianglePlane.IsFrontFacingTo(colPackage->normalizedVelocity))
		return;

	// Get interval of Plane intersection:
	float t0, t1;
	bool embeddedInPlane = false;

	// Calculate the signed distance from sphere position to triangle Plane
	float signedDistToTrianglePlane = trianglePlane.SignedDistanceTo(colPackage->basePoint);

	// cache this as weТre going to use it a few times below:
	float normalDotVelocity = glm::dot(trianglePlane.normal, colPackage->velocity);
	// if sphere is travelling parrallel to the Plane:
	if (normalDotVelocity == 0.0f) 
	{
		if (fabs(signedDistToTrianglePlane) >= 1.0f)
		{
			// Sphere is not embedded in Plane.
			// No collision possible:
			return;
		}
		else
		{
			// sphere is embedded in Plane.
			// It intersects in the whole range [0..1]
			embeddedInPlane = true;
			t0 = 0.0f;
			t1 = 1.0f;
		}
	}
	else 
	{
		// N dot D is not 0. Calculate intersection interval:
		t0 = (-1.0f - signedDistToTrianglePlane) / normalDotVelocity;
		t1 = ( 1.0f - signedDistToTrianglePlane) / normalDotVelocity;

		// Swap so t0 < t1
		if (t0 > t1) 
		{
			float temp = t1;
			t1 = t0;
			t0 = temp;
		}

		// Check that at least one result is within range:
		if (t0 > 1.0f || t1 < 0.0f)
		{
			// Both t values are outside values [0,1]
			// No collision possible:
			return;
		}
		// Clamp to [0,1]
		if (t0 < 0.0f) t0 = 0.0f;
		if (t1 < 0.0f) t1 = 0.0f;
		if (t0 > 1.0f) t0 = 1.0f;
		if (t1 > 1.0f) t1 = 1.0f;
	}

	// OK, at this point we have two time values t0 and t1
	// between which the swept sphere intersects with the
	// triangle Plane. If any collision is to occur it must
	// happen within this interval.
	glm::vec3 collisionPoint;
	bool foundCollison = false;
	float t = 1.0f;

	// First we check for the easy case - collision inside
	// the triangle. If this happens it must be at time t0
	// as this is when the sphere rests on the front side
	// of the triangle Plane. Note, this can only happen if
	// the sphere is not embedded in the triangle Plane.
	if (!embeddedInPlane)
	{
		glm::vec3 PlaneIntersectionPoint = (colPackage->basePoint - trianglePlane.normal) + t0 * colPackage->velocity;

		if (Collisions::CheckPointInTriangle( p1, p2, p3, PlaneIntersectionPoint))
		{
			foundCollison = true;
			t = t0;
			collisionPoint = PlaneIntersectionPoint;
		}
	}
	// if we havenТt found a collision already weТll have to
	// sweep sphere against points and edges of the triangle.
	// Note: A collision inside the triangle (the check above)
	// will always happen before a vertex or edge collision!
	// This is why we can skip the swept test if the above
	// gives a collision!

	if (foundCollison == false) 
	{
		// some commonly used terms:
		glm::vec3 velocity = colPackage->velocity;
		glm::vec3 base = colPackage->basePoint;
		float velocitySquaredLength = glm::dot(velocity, velocity);
		float a, b, c; // Params for equation
		float newT;

		// For each vertex or edge a quadratic equation have to
		// be solved. We parameterize this equation as
		// a*t^2 + b*t + c = 0 and below we calculate the
		// parameters a,b and c for each test.

		// Check against points:
		a = velocitySquaredLength;

		// P1
		b = 2.0f * (glm::dot(velocity, base - p1));
		c = glm::dot(p1 - base, p1 - base) - 1.0f;
		if (Collisions::GetLowestRoot(a, b, c, t, &newT)) 
		{
			t = newT;
			foundCollison = true;
			collisionPoint = p1;
		}

		// P2
		if (!foundCollison)
		{
			b = 2.0f * (glm::dot(velocity, base - p2));
			c = glm::dot(p2 - base, p2 - base) - 1.0f;
			if (Collisions::GetLowestRoot(a, b, c, t, &newT))
			{
				t = newT;
				foundCollison = true;
				collisionPoint = p2;
			}
		}
		
		// P3
		if (!foundCollison)
		{
			b = 2.0f * (glm::dot(velocity, base - p3));
			c = glm::dot(p3 - base, p3 - base) - 1.0f;
			if (Collisions::GetLowestRoot(a, b, c, t, &newT))
			{
				t = newT;
				foundCollison = true;
				collisionPoint = p3;
			}
		}

		// Check agains edges:

		// p1 -> p2:
		glm::vec3 edge = p2 - p1;
		glm::vec3 baseToVertex = p1 - base;
		float edgeSquaredLength = glm::dot(edge, edge);
		float edgeDotVelocity = glm::dot(edge, velocity);
		float edgeDotBaseToVertex = glm::dot(edge, baseToVertex);

		// Calculate parameters for equation
		a = edgeSquaredLength * -velocitySquaredLength + edgeDotVelocity * edgeDotVelocity;
		b = edgeSquaredLength * (2.0f * glm::dot(velocity, baseToVertex)) - 2.0f * edgeDotVelocity * edgeDotBaseToVertex;
		c = edgeSquaredLength * (1.0f - glm::dot(baseToVertex, baseToVertex)) + edgeDotBaseToVertex * edgeDotBaseToVertex;

		// Does the swept sphere collide against infinite edge?
		if (Collisions::GetLowestRoot(a, b, c, t, &newT))
		{
			// Check if intersection is within line segment:
			float f = (edgeDotVelocity * newT - edgeDotBaseToVertex) / edgeSquaredLength;
			if (f >= 0.0f && f <= 1.0f) 
			{
				// intersection took place within segment.
				t = newT;
				foundCollison = true;
				collisionPoint = p1 + f * edge;
			}
		}

		// p2 -> p3:
		edge = p3 - p2;
		baseToVertex = p2 - base;
		edgeSquaredLength = glm::dot(edge, edge);
		edgeDotVelocity = glm::dot(edge, velocity);
		edgeDotBaseToVertex = glm::dot(edge, baseToVertex);

		// calculate params for equation
		a = edgeSquaredLength * -velocitySquaredLength + edgeDotVelocity * edgeDotVelocity;
		b = edgeSquaredLength * (2.0f * glm::dot(velocity, baseToVertex)) - 2.0f * edgeDotVelocity * edgeDotBaseToVertex;
		c = edgeSquaredLength * (1.0f - glm::dot(baseToVertex, baseToVertex)) + edgeDotBaseToVertex * edgeDotBaseToVertex;

		// do we collide against infinite edge
		if (Collisions::GetLowestRoot(a, b, c, t, &newT)) 
		{
			// check if intersect is within line segment
			float f = (edgeDotVelocity * newT - edgeDotBaseToVertex) / edgeSquaredLength;
			if (f >= 0.0f && f <= 1.0f) 
			{
				t = newT;
				foundCollison = true;
				collisionPoint = p2 + f * edge;
			}
		}

		// p3 -> p1:
		edge = p1 - p3;
		baseToVertex = p3 - base;
		edgeSquaredLength = glm::dot(edge, edge);
		edgeDotVelocity = glm::dot(edge, velocity);
		edgeDotBaseToVertex = glm::dot(edge, baseToVertex);

		// calculate params for equation
		a = edgeSquaredLength * -velocitySquaredLength + edgeDotVelocity * edgeDotVelocity;
		b = edgeSquaredLength * (2.0f * glm::dot(velocity, baseToVertex)) - 2.0f * edgeDotVelocity * edgeDotBaseToVertex;
		c = edgeSquaredLength * (1.0f - glm::dot(baseToVertex, baseToVertex)) + edgeDotBaseToVertex * edgeDotBaseToVertex;

		// do we collide against infinite edge
		if (Collisions::GetLowestRoot(a, b, c, t, &newT))
		{
			// check if intersect is within line segment
			float f = (edgeDotVelocity * newT - edgeDotBaseToVertex) / edgeSquaredLength;
			if (f >= 0.0f && f <= 1.0f) 
			{
				t = newT;
				foundCollison = true;
				collisionPoint = p3 + f * edge;
			}
		}
	}
	// Set result:
	if (foundCollison == true)
	{
		// distance to collision: ТtТ is time of collision
		float distToCollision = t * glm::length(colPackage->velocity);
		// Does this triangle qualify for the closest hit?
		// it does if itТs the first hit or the closest
		if (colPackage->foundCollision == false || distToCollision < colPackage->nearestDistance) 
		{
			// Collision information nessesary for sliding
			colPackage->nearestDistance = distToCollision;
			colPackage->intersectionPoint = collisionPoint;
			colPackage->foundCollision = true;
			colPackage->t = t;
			colPackage->plane = trianglePlane;
			colPackage->a = p1;
			colPackage->b = p2;
			colPackage->c = p3;
		}
	}
}

class CharacterEntity
{
public:
	CharacterEntity(g3d::Model* model, glm::vec3 radius);
	CharacterEntity(std::vector<g3d::Model*> models, glm::vec3 radius);
	void Update(float dt);
	void CheckCollision();

	void CollideAndSlide(const glm::vec3& gravity);

	glm::vec3 CollideWithWorld(const glm::vec3& pos, const glm::vec3& velocity);

	// new
	void CollideAndSlide2();
	void CollideWithWorld2(glm::vec3& pos, glm::vec3& velocity);
	void CheckGrounded();
	void Update2(float dt);

	glm::vec3 position, velocity, radius;
	CollisionPacket collisionPackage;
	std::vector<g3d::Model*> models;
	int grounded;
};

CharacterEntity::CharacterEntity(g3d::Model* model, glm::vec3 radius)
{
	this->radius = radius;
	position = glm::vec3(0.0f);
	velocity = glm::vec3(0.0f);

	this->models.push_back(model);
	grounded = 0;
}

CharacterEntity::CharacterEntity(std::vector<g3d::Model*> models, glm::vec3 radius)
{
	this->radius = radius;
	position = glm::vec3(0.0f);
	velocity = glm::vec3(0.0f);

	this->models = models;
	grounded = 0;
}

void CharacterEntity::CollideAndSlide(const glm::vec3& gravity)
{
#if 1
	// Do collision detection:
	collisionPackage.R3Position = position;
	collisionPackage.R3Velocity = velocity;
	collisionPackage.eRadius = radius;

	// calculate position and velocity in eSpace
	glm::vec3 eSpacePosition = collisionPackage.R3Position / collisionPackage.eRadius;
	glm::vec3 Velocity = collisionPackage.R3Velocity / collisionPackage.eRadius;
	// no gravity
	Velocity.y = 0.0f;

	// Iterate until we have our final position.
	collisionPackage.collisionRecursionDepth = 0;

	int g = grounded;
	glm::vec3 finalPosition = CollideWithWorld(eSpacePosition, Velocity);
	grounded = g;

	// Add gravity pull:

	// Set the new R3 position (convert back from eSpace to R3)
	collisionPackage.R3Position = finalPosition * collisionPackage.eRadius;
	collisionPackage.R3Velocity = gravity;

	// convert velocity to e-space
	Velocity = gravity / collisionPackage.eRadius;

	// gravity iteration
	collisionPackage.collisionRecursionDepth = 0;
	finalPosition = CollideWithWorld(finalPosition, Velocity);

	// finally set entity position
	position = finalPosition * collisionPackage.eRadius;
#else
	// Do collision detection:
	collisionPackage.R3Position = position;
	collisionPackage.R3Velocity = velocity;
	collisionPackage.eRadius = radius;

	// y velocity in a seperate pass
	glm::vec3 Gravity = glm::vec3{ 0.0f };
	Gravity.y = collisionPackage.R3Velocity.y;
	//collisionPackage.R3Velocity.y = 0.0f;

	// lets get e-spacey?
	// calculate position and velocity in eSpace
	glm::vec3 eSpacePosition = collisionPackage.R3Position / collisionPackage.eRadius;
	glm::vec3 eSpaceVelocity = collisionPackage.R3Velocity / collisionPackage.eRadius;

	// Iterate until we have our final position.
	collisionPackage.collisionRecursionDepth = 0;

	int g = grounded;
	glm::vec3 finalPosition = CollideWithWorld(eSpacePosition, velocity);
	grounded = g;


#if 1
	// Set the new R3 position (convert back from eSpace to R3)
	collisionPackage.R3Position = finalPosition * collisionPackage.eRadius;
	collisionPackage.R3Velocity = gravity;

	// convert velocity to e-space
	eSpaceVelocity = gravity / collisionPackage.eRadius;

	// gravity iteration
	collisionPackage.collisionRecursionDepth = 0;
	finalPosition = CollideWithWorld(finalPosition, velocity);

	// finally set entity position
	position = finalPosition * collisionPackage.eRadius;
#else
	// do gravity iteration
	collisionPackage.R3Velocity = Gravity;
	eSpaceVelocity = Gravity / collisionPackage.eRadius;
	/*glm::vec3 finalPosition = */CollideWithWorld(eSpacePosition, velocity);

	// finally set entity position & velocity
	position = eSpacePosition * collisionPackage.eRadius;
#endif
#endif
}

inline void CharacterEntity::CollideAndSlide2()
{
	// Do collision detection:
	collisionPackage.R3Position = position;
	collisionPackage.R3Velocity = velocity;
	collisionPackage.eRadius = radius;

	// y velocity in a seperate pass
	glm::vec3 gravity = glm::vec3{ 0.0f };
	gravity.y = collisionPackage.R3Velocity.y;
	collisionPackage.R3Velocity.y = 0.0f;

	// calculate position and velocity in eSpace
	glm::vec3 eSpacePosition = collisionPackage.R3Position / collisionPackage.eRadius;
	glm::vec3 eVelocity = collisionPackage.R3Velocity / collisionPackage.eRadius;

	// do velocity iteration
	CollideWithWorld2(eSpacePosition, eVelocity);
	
	// do gravity iteration
	collisionPackage.R3Velocity = gravity;
	eVelocity = collisionPackage.R3Velocity / collisionPackage.eRadius;
	CollideWithWorld2(eSpacePosition, eVelocity);
		
	// finally set entity position & velocity
	position = eSpacePosition * collisionPackage.eRadius;
}

inline void CharacterEntity::CollideWithWorld2(glm::vec3& e_position, glm::vec3& e_velocity)
{
	glm::vec3 dest = e_position + e_velocity;
	glm::vec3 src = e_position;

	Plane3 first_plane;

	// check for collision
	glm::vec3 temp;
	for (int i = 0; i < 3; i++)
	{
		// setup coll packet
		collisionPackage.velocity = e_velocity;
		collisionPackage.normalizedVelocity = glm::normalize(e_velocity);
		collisionPackage.basePoint = e_position;
		collisionPackage.eRadius = radius;
		collisionPackage.foundCollision = false;
		collisionPackage.nearestDistance = FLT_MAX;
		collisionPackage.t = 0.0f;

		// Check for collision (calls the collision routines)
	// Application specific!!
		CheckCollision();
		CheckGrounded();

		// If no collision we just move along the velocity
		if (collisionPackage.foundCollision == false)
		{
			e_position = dest; // TODO: переделать на возврат
			return;
		}

		// point touching tri
		glm::vec3 touch_point = e_position + e_velocity * collisionPackage.t;

		float dist = sqrtf(glm::dot(e_velocity, e_velocity)) * collisionPackage.t;
		float short_dist = Max(dist - 1.0f, 0.0f);

		e_position = e_position + glm::normalize(e_velocity) * short_dist;

		// Determine the sliding Plane
		glm::vec3 slidePlaneOrigin = collisionPackage.intersectionPoint; // use intersect point as origin
		// normal = touch_point - intersect_point
		// dont use normal from packet.plane.normal!
		glm::vec3 slidePlaneNormal = glm::normalize(touch_point - collisionPackage.intersectionPoint);

		if (i == 0)
		{
			float long_radius = 1.0 + VERY_CLOSE_DIST;
			first_plane = Plane3(slidePlaneOrigin, slidePlaneNormal);

			float dist_to_plane = first_plane.SignedDistanceTo(dest) - long_radius;

			dest = dest - first_plane.normal * dist_to_plane;
			e_velocity = dest - e_position;
		}
		else if (i == 1)
		{
			Plane3 second_plane(slidePlaneOrigin, slidePlaneNormal);

			glm::vec3 crease = glm::cross(first_plane.normal, second_plane.normal);
			float dis = glm::dot(dest - e_position, crease);
			crease = glm::normalize(crease);

			e_velocity = crease * dis;
			dest = e_position + e_velocity;
		}
	}

	e_position = dest;
}

void CharacterEntity::CheckGrounded()
{
	if (!collisionPackage.foundCollision)
		return;

	glm::vec3 axis = glm::vec3{ 0.0f, 1.0f, 0.0f };

	glm::vec3 a = collisionPackage.a * radius;
	glm::vec3 b = collisionPackage.b * radius;
	glm::vec3 c = collisionPackage.c * radius;

	Plane3 plane(a, b, c);
	float f = glm::dot(plane.normal, axis);
	if (f >= SLOPE_WALK_ANGLE)
		grounded = 1;
}

inline void CharacterEntity::Update2(float dt)
{
	glm::vec3 xz = glm::vec3(velocity.x, 0.0f, velocity.z);
	// prevent sliding while standing still on a slope
	// change < 0.0f to > 0.0f if inverting y axis

	if (grounded && fabs(sqrtf(glm::dot(xz, xz))) < 0.1f && velocity.y < 0.0f)
		velocity.y = 0.0f;
	else
		grounded = 0;

	dt = dt / 5.0;

	velocity = velocity * dt;
	for (int i = 0; i < 5; i++)
		CollideAndSlide2();

	velocity = position - collisionPackage.R3Position;
	velocity = velocity * 1.0f / dt;
}

glm::vec3 CharacterEntity::CollideWithWorld(const glm::vec3& pos, const glm::vec3& vel)
{
	// All hard-coded distances in this function is
	// scaled to fit the setting above..
	float unitScale = unitsPerMeter / 100.0f;
	float veryCloseDistance = 0.0000005f * unitScale;

	// do we need to worry?
	if (collisionPackage.collisionRecursionDepth > 5)
		return pos;

	// Ok, we need to worry:
	collisionPackage.velocity = vel;
	collisionPackage.normalizedVelocity = glm::normalize(vel);
	collisionPackage.basePoint = pos;
	collisionPackage.foundCollision = false;
	collisionPackage.nearestDistance = FLT_MAX;

	// Check for collision (calls the collision routines)
	// Application specific!!
	CheckCollision();

	// If no collision we just move along the velocity
	if (collisionPackage.foundCollision == false)
	{
		return pos + vel;
	}

	// *** Collision occured ***

	// The original destination point
	glm::vec3 destinationPoint = pos + vel;
	glm::vec3 newBasePoint = pos;
	// only update if we are not already very close
	// and if so we only move very close to intersection..not
	// to the exact spot.
	if (collisionPackage.nearestDistance >= veryCloseDistance)
	{
		glm::vec3 v = (float)Min(glm::length(vel), collisionPackage.nearestDistance - veryCloseDistance) * vel;
		// TODO: может это вместо выше v.SetLength(collisionPackage->nearestDistance - veryCloseDistance);
		newBasePoint = collisionPackage.basePoint + v;

		// Adjust polygon intersection point (so sliding
		// Plane will be unaffected by the fact that we
		// move slightly less than collision tells us)
		v = glm::normalize(v);
		collisionPackage.intersectionPoint -= veryCloseDistance * v;
	}

	// Determine the sliding Plane
	glm::vec3 slidePlaneOrigin = collisionPackage.intersectionPoint;
	glm::vec3 slidePlaneNormal = newBasePoint - collisionPackage.intersectionPoint;
	slidePlaneNormal = glm::normalize(slidePlaneNormal);

	Plane3 slidingPlane(slidePlaneOrigin, slidePlaneNormal);

	glm::vec3 newDestinationPoint = destinationPoint - (float)slidingPlane.SignedDistanceTo(destinationPoint) * slidePlaneNormal;

	// Generate the slide vectpr, which will become our new
	// velocity vector for the next iteration
	glm::vec3 newVelocityVector = newDestinationPoint - collisionPackage.intersectionPoint;

	if (collisionPackage.intersectionPoint.y <= pos.y - collisionPackage.eRadius.y + 0.1f && vel.y <= 0.0f)
	{
		glm::vec3 a = collisionPackage.a * radius;
		glm::vec3 b = collisionPackage.b * radius;
		glm::vec3 c = collisionPackage.c * radius;
		Plane3 plane(a, b, c);
		glm::vec3 axis = { 0.0f, 1.0f, 0.0f };
		float f = glm::dot(plane.normal, axis);
		if (f >= SLOPE_WALK_ANGLE)
			grounded = 1;
	}

	// Recurse:

	// dont recurse if the new velocity is very small
	if (glm::length(newVelocityVector) < veryCloseDistance)
	{
		return newBasePoint;
	}

	collisionPackage.collisionRecursionDepth++;

	return CollideWithWorld(newBasePoint, newVelocityVector);
}

void CharacterEntity::CheckCollision()
{
	// сначала тестим все объекты на попадание в AABB
	// в выбранных получаем список треугольников, при этом каждый треугольник хранит (или вычисл€етс€) свой размер - дл€ простоты в виде AABB.
	// теперь провер€ем эти треугольники в попадание зоны јјЅЅ объекта-игрока.
	// и вот с полученными уже обрабатываем коллизии



	// check collision against triangles

	// TODO: если не сработает, то получать сабмеши. “акже надо подумать над вершинами и индексами
	for (size_t i = 0; i < models.size(); i++) 
	{
		auto poly = models[i]->GetPoly();
		//auto poly = modelPoly;

		for (size_t j = 0; j < poly.verts.size(); j+=3)
		{
			glm::vec3 a = poly.verts[j + 0] / collisionPackage.eRadius;
			glm::vec3 b = poly.verts[j + 1] / collisionPackage.eRadius;
			glm::vec3 c = poly.verts[j + 2] / collisionPackage.eRadius;
			CheckCollisionsTriangle(&collisionPackage, a, b, c);
		}
	}
}

void CharacterEntity::Update(float dt)
{
	grounded = 0;
	glm::vec3 gravity = { 0.0f, this->velocity.y, 0.0f };
	CollideAndSlide(gravity);

	glm::vec3 xz = glm::vec3{ velocity.x, 0.0f, velocity.z };

	// prevent sliding while standing still on a slope change < 0.0f to > 0.0f if inverting y axis
	if (grounded /* && fabs(glm::length(xz)) < 0.1f && velocity.y < 0.0f*/)
		velocity.y = 0.0f;
	//else
	//	grounded = 0;
}

CharacterEntity* entity;
// size of collision ellipse, experiment with this to change fidelity of detection
static glm::vec3 boundingEllipse = { 0.5f, 1.0f, 0.5f };

void InitTest()
{
	// Init Camera
	{
		ncamera.Teleport(0, 3, -6);
		ncamera.LookAt(glm::vec3(0, 0, 0));
		ncamera.Enable();
		ncamera.m_speed = 5;
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
	}

	// Load character
	{

		entity = new CharacterEntity(&model, boundingEllipse);// initialize player infront of model
		entity->position[1] = 10.0f;
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

	shader.Bind();

	shader.SetUniform(viewUniform, ncamera.m_view);
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());

	shader.SetUniform(worldUniform, transform.GetWorld());
	model.Draw();
	//vao.Draw();

	//DebugDraw::DrawGrid(0);

	// character
	{

		glm::vec3 ijklec = tempMath::scale3(glm::vec3(IsKeyboardKeyDown(KEY_L) - IsKeyboardKeyDown(KEY_J), 0, IsKeyboardKeyDown(KEY_I) - IsKeyboardKeyDown(KEY_K)), 500.0f * deltaTime);
		entity->velocity.x = ijklec.x;
		entity->velocity.y = -300.0f * deltaTime;
		entity->velocity.z = ijklec.z;

		entity->Update2(deltaTime);

		DebugDraw::DrawCapsule(
			glm::vec3(entity->position.x, entity->position.y -0.5, entity->position.z),
			glm::vec3(entity->position.x, entity->position.y + 0.5, entity->position.z),
			0.5f, RED);
	}

	DebugDraw::Flush(ncamera);
}