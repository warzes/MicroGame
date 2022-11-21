#pragma once

constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 vTexCoord;

void main()
{
	gl_Position = uProjection * uView * uWorld * vec4(vPos, 1.0);
	vTexCoord = aTexCoord;
}
)";
constexpr const char* fragment_shader_text = R"(
#version 330 core

in vec2 vTexCoord;

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

g3d::Model model;
g3d::Material material;
Transform transform;

Camera ncamera;

Poly polyModel;


namespace temp {

	// Base struct for all collision shapes
	struct Collider {
		glm::vec3    pos;            //origin in world space
		glm::mat3    matRS;          //rotation/scale component of model matrix
		glm::mat3    matRS_inverse;
		virtual glm::vec3 support(glm::vec3 dir) = 0;
	};

	//BBox: AABB + Orientation matrix
	struct BBox : Collider {
		glm::vec3 min, max; //Assume these are axis aligned!

		glm::vec3 support(glm::vec3 dir) {
			dir = matRS_inverse * dir; //find support in model space

			glm::vec3 result;
			result.x = (dir.x > 0) ? max.x : min.x;
			result.y = (dir.y > 0) ? max.y : min.y;
			result.z = (dir.z > 0) ? max.z : min.z;

			return matRS * result + pos; //convert support to world space
		}
	};

	//Sphere: NB Does not use RS matrix, scale the radius directly!
	struct Sphere : Collider {
		float r;

		glm::vec3 support(glm::vec3 dir) {
			return glm::normalize(dir) * r + pos;
		}
	};

	//Cylinder: Height-aligned with y-axis (rotate using matRS)
	struct Cylinder : Collider {
		float r, y_base, y_cap;

		glm::vec3 support(glm::vec3 dir) 
		{
			dir = matRS_inverse * dir; //find support in model space

			glm::vec3 dir_xz = glm::vec3(dir.x, 0, dir.z);
			glm::vec3 result = glm::normalize(dir_xz) * r;
			result.y = (dir.y > 0) ? y_cap : y_base;

			return matRS * result + pos; //convert support to world space
		}
	};

	//Capsule: Height-aligned with y-axis
	struct Capsule : Collider
	{
		float r, y_base, y_cap;

		glm::vec3 support(glm::vec3 dir) {
			dir = matRS_inverse * dir; //find support in model space

			glm::vec3 result = glm::normalize(dir) * r;
			result.y += (dir.y > 0) ? y_cap : y_base;

			return matRS * result + pos; //convert support to world space
		}
	};

	struct TriangleCollider : Collider {
		glm::vec3 points[3];
		glm::vec3 normal;

		glm::vec3 support(glm::vec3 dir) {
			//Find which triangle vertex is furthest along dir
			float dot0 = glm::dot(points[0], dir);
			float dot1 = glm::dot(points[1], dir);
			float dot2 = glm::dot(points[2], dir);
			glm::vec3 furthest_point = points[0];
			if (dot1 > dot0) {
				furthest_point = points[1];
				if (dot2 > dot1)
					furthest_point = points[2];
			}
			else if (dot2 > dot0) {
				furthest_point = points[2];
			}

			//fake some depth behind triangle so we have volume
			if (dot(dir, normal) < 0) furthest_point -= normal;

			return furthest_point;
		}
	};

	//Polytope: Just a set of points
	struct Polytope : Collider {
		float* points;    //(x0 y0 z0 x1 y1 z1 etc)
		int     num_points;

		//Dumb O(n) support function, just brute force check all points
		glm::vec3 support(glm::vec3 dir) {
			dir = matRS_inverse * dir;

			glm::vec3 furthest_point = glm::vec3(points[0], points[1], points[2]);
			float max_dot = glm::dot(furthest_point, dir);

			for (int i = 3; i < num_points * 3; i += 3) {
				glm::vec3 v = glm::vec3(points[i], points[i + 1], points[i + 2]);
				float d = glm::dot(v, dir);
				if (d > max_dot) {
					max_dot = d;
					furthest_point = v;
				}
			}
			glm::vec3 result = matRS * furthest_point + pos; //convert support to world space
			return result;
		}
	};

// implementation of the Gilbert-Johnson-Keerthi intersection algorithm
//and the Expanding Polytope Algorithm
//Most useful references (Huge thanks to all the authors):

// "Implementing GJK" by Casey Muratori:
// The best description of the algorithm from the ground up
// https://www.youtube.com/watch?v=Qupqu1xe7Io

// "Implementing a GJK Intersection Query" by Phill Djonov
// Interesting tips for implementing the algorithm
// http://vec3.ca/gjk/implementation/

// "GJK Algorithm 3D" by Sergiu Craitoiu
// Has nice diagrams to visualise the tetrahedral case
// http://in2gpu.com/2014/05/18/gjk-algorithm-3d/

// "GJK + Expanding Polytope Algorithm - Implementation and Visualization"
// Good breakdown of EPA with demo for visualisation
// https://www.youtube.com/watch?v=6rgiPrzqt9w

    //Returns true if two colliders are intersecting. Has optional Minimum Translation Vector output param;
//If supplied the EPA will be used to find the vector to separate coll1 from coll2
    bool gjk(Collider* coll1, Collider* coll2, glm::vec3* mtv = NULL);
    //Internal functions used in the GJK algorithm
    void update_simplex3(glm::vec3& a, glm::vec3& b, glm::vec3& c, glm::vec3& d, int& simp_dim, glm::vec3& search_dir);
    bool update_simplex4(glm::vec3& a, glm::vec3& b, glm::vec3& c, glm::vec3& d, int& simp_dim, glm::vec3& search_dir);
    //Expanding Polytope Algorithm. Used to find the mtv of two intersecting 
    //colliders using the final simplex obtained with the GJK algorithm
    glm::vec3 EPA(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, Collider* coll1, Collider* coll2);

#define GJK_MAX_NUM_ITERATIONS 64

    bool gjk(Collider* coll1, Collider* coll2, glm::vec3* mtv) {
        glm::vec3 a, b, c, d; //Simplex: just a set of points (a is always most recently added)
        glm::vec3 search_dir = coll1->pos - coll2->pos; //initial search direction between colliders

        //Get initial point for simplex
        c = coll2->support(search_dir) - coll1->support(-search_dir);
        search_dir = -c; //search in direction of origin

        //Get second point for a line segment simplex
        b = coll2->support(search_dir) - coll1->support(-search_dir);

        if (dot(b, search_dir) < 0) { return false; }//we didn't reach the origin, won't enclose it

        search_dir = cross(cross(c - b, -b), c - b); //search perpendicular to line segment towards origin
        if (search_dir == glm::vec3(0, 0, 0)) { //origin is on this line segment
            //Apparently any normal search vector will do?
            search_dir = glm::cross(c - b, glm::vec3(1, 0, 0)); //normal with x-axis
            if (search_dir == glm::vec3(0, 0, 0)) search_dir = glm::cross(c - b, glm::vec3(0, 0, -1)); //normal with z-axis
        }
        int simp_dim = 2; //simplex dimension

        for (int iterations = 0; iterations < GJK_MAX_NUM_ITERATIONS; iterations++)
        {
            a = coll2->support(search_dir) - coll1->support(-search_dir);
            if (dot(a, search_dir) < 0) { return false; }//we didn't reach the origin, won't enclose it

            simp_dim++;
            if (simp_dim == 3) {
                update_simplex3(a, b, c, d, simp_dim, search_dir);
            }
            else if (update_simplex4(a, b, c, d, simp_dim, search_dir)) {
                if (mtv) *mtv = EPA(a, b, c, d, coll1, coll2);
                return true;
            }
        }//endfor
        return false;
    }

    //Triangle case
    void update_simplex3(glm::vec3& a, glm::vec3& b, glm::vec3& c, glm::vec3& d, int& simp_dim, glm::vec3& search_dir) 
    {
        /* Required winding order:
        //  b
        //  | \
        //  |   \
        //  |    a
        //  |   /
        //  | /
        //  c
        */
        glm::vec3 n = glm::cross(b - a, c - a); //triangle's normal
        glm::vec3 AO = -a; //direction to origin

        //Determine which feature is closest to origin, make that the new simplex

        simp_dim = 2;
        if (glm::dot(glm::cross(b - a, n), AO) > 0) { //Closest to edge AB
            c = a;
            //simp_dim = 2;
            search_dir = glm::cross(glm::cross(b - a, AO), b - a);
            return;
        }
        if (glm::dot(glm::cross(n, c - a), AO) > 0) { //Closest to edge AC
            b = a;
            //simp_dim = 2;
            search_dir = glm::cross(glm::cross(c - a, AO), c - a);
            return;
        }

        simp_dim = 3;
        if (glm::dot(n, AO) > 0) { //Above triangle
            d = c;
            c = b;
            b = a;
            //simp_dim = 3;
            search_dir = n;
            return;
        }
        //else //Below triangle
        d = b;
        b = a;
        //simp_dim = 3;
        search_dir = -n;
        return;
    }

    //Tetrahedral case
    bool update_simplex4(glm::vec3& a, glm::vec3& b, glm::vec3& c, glm::vec3& d, int& simp_dim, glm::vec3& search_dir) {
        // a is peak/tip of pyramid, BCD is the base (counterclockwise winding order)
        //We know a priori that origin is above BCD and below a

        //Get normals of three new faces
        glm::vec3 ABC = cross(b - a, c - a);
        glm::vec3 ACD = cross(c - a, d - a);
        glm::vec3 ADB = cross(d - a, b - a);

        glm::vec3 AO = -a; //dir to origin
        simp_dim = 3; //hoisting this just cause

        //Plane-test origin with 3 faces
        /*
        // Note: Kind of primitive approach used here; If origin is in front of a face, just use it as the new simplex.
        // We just go through the faces sequentially and exit at the first one which satisfies dot product. Not sure this
        // is optimal or if edges should be considered as possible simplices? Thinking this through in my head I feel like
        // this method is good enough. Makes no difference for AABBS, should test with more complex colliders.
        */
        if (glm::dot(ABC, AO) > 0) { //In front of ABC
            d = c;
            c = b;
            b = a;
            search_dir = ABC;
            return false;
        }
        if (glm::dot(ACD, AO) > 0) { //In front of ACD
            b = a;
            search_dir = ACD;
            return false;
        }
        if (glm::dot(ADB, AO) > 0) { //In front of ADB
            c = d;
            d = b;
            b = a;
            search_dir = ADB;
            return false;
        }

        //else inside tetrahedron; enclosed!
        return true;

        //Note: in the case where two of the faces have similar normals,
        //The origin could conceivably be closest to an edge on the tetrahedron
        //Right now I don't think it'll make a difference to limit our new simplices
        //to just one of the faces, maybe test it later.
    }

    //Expanding Polytope Algorithm
    //Find minimum translation vector to resolve collision
#define EPA_TOLERANCE 0.0001
#define EPA_MAX_NUM_FACES 64
#define EPA_MAX_NUM_LOOSE_EDGES 32
#define EPA_MAX_NUM_ITERATIONS 64
    glm::vec3 EPA(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, Collider* coll1, Collider* coll2)
    {
        glm::vec3 faces[EPA_MAX_NUM_FACES][4]; //Array of faces, each with 3 verts and a normal

        //Init with final simplex from GJK
        faces[0][0] = a;
        faces[0][1] = b;
        faces[0][2] = c;
        faces[0][3] = glm::normalize(glm::cross(b - a, c - a)); //ABC
        faces[1][0] = a;
        faces[1][1] = c;
        faces[1][2] = d;
        faces[1][3] = glm::normalize(glm::cross(c - a, d - a)); //ACD
        faces[2][0] = a;
        faces[2][1] = d;
        faces[2][2] = b;
        faces[2][3] = glm::normalize(glm::cross(d - a, b - a)); //ADB
        faces[3][0] = b;
        faces[3][1] = d;
        faces[3][2] = c;
        faces[3][3] = glm::normalize(glm::cross(d - b, c - b)); //BDC

        int num_faces = 4;
        int closest_face;

        for (int iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++) {
            //Find face that's closest to origin
            float min_dist = glm::dot(faces[0][0], faces[0][3]);
            closest_face = 0;
            for (int i = 1; i < num_faces; i++) {
                float dist = glm::dot(faces[i][0], faces[i][3]);
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_face = i;
                }
            }

            //search normal to face that's closest to origin
            glm::vec3 search_dir = faces[closest_face][3];
            glm::vec3 p = coll2->support(search_dir) - coll1->support(-search_dir);

            if (glm::dot(p, search_dir) - min_dist < EPA_TOLERANCE) {
                //Convergence (new point is not significantly further from origin)
                return faces[closest_face][3] * glm::dot(p, search_dir); //dot vertex with normal to resolve collision along normal!
            }

            glm::vec3 loose_edges[EPA_MAX_NUM_LOOSE_EDGES][2]; //keep track of edges we need to fix after removing faces
            int num_loose_edges = 0;

            //Find all triangles that are facing p
            for (int i = 0; i < num_faces; i++)
            {
                if (glm::dot(faces[i][3], p - faces[i][0]) > 0) //triangle i faces p, remove it
                {
                    //Add removed triangle's edges to loose edge list.
                    //If it's already there, remove it (both triangles it belonged to are gone)
                    for (int j = 0; j < 3; j++) //Three edges per face
                    {
                        glm::vec3 current_edge[2] = { faces[i][j], faces[i][(j + 1) % 3] };
                        bool found_edge = false;
                        for (int k = 0; k < num_loose_edges; k++) //Check if current edge is already in list
                        {
                            if (loose_edges[k][1] == current_edge[0] && loose_edges[k][0] == current_edge[1]) {
                                //Edge is already in the list, remove it
                                //THIS ASSUMES EDGE CAN ONLY BE SHARED BY 2 TRIANGLES (which should be true)
                                //THIS ALSO ASSUMES SHARED EDGE WILL BE REVERSED IN THE TRIANGLES (which 
                                //should be true provided every triangle is wound CCW)
                                loose_edges[k][0] = loose_edges[num_loose_edges - 1][0]; //Overwrite current edge
                                loose_edges[k][1] = loose_edges[num_loose_edges - 1][1]; //with last edge in list
                                num_loose_edges--;
                                found_edge = true;
                                k = num_loose_edges; //exit loop because edge can only be shared once
                            }
                        }//endfor loose_edges

                        if (!found_edge) { //add current edge to list
                            // assert(num_loose_edges<EPA_MAX_NUM_LOOSE_EDGES);
                            if (num_loose_edges >= EPA_MAX_NUM_LOOSE_EDGES) break;
                            loose_edges[num_loose_edges][0] = current_edge[0];
                            loose_edges[num_loose_edges][1] = current_edge[1];
                            num_loose_edges++;
                        }
                    }

                    //Remove triangle i from list
                    faces[i][0] = faces[num_faces - 1][0];
                    faces[i][1] = faces[num_faces - 1][1];
                    faces[i][2] = faces[num_faces - 1][2];
                    faces[i][3] = faces[num_faces - 1][3];
                    num_faces--;
                    i--;
                }//endif p can see triangle i
            }//endfor num_faces

            //Reconstruct polytope with p added
            for (int i = 0; i < num_loose_edges; i++)
            {
                // assert(num_faces<EPA_MAX_NUM_FACES);
                if (num_faces >= EPA_MAX_NUM_FACES) break;
                faces[num_faces][0] = loose_edges[i][0];
                faces[num_faces][1] = loose_edges[i][1];
                faces[num_faces][2] = p;
                faces[num_faces][3] = glm::normalize(cross(loose_edges[i][0] - loose_edges[i][1], loose_edges[i][0] - p));

                //Check for wrong normal to maintain CCW winding
                float bias = 0.000001; //in case dot result is only slightly < 0 (because origin is on face)
                if (glm::dot(faces[num_faces][0], faces[num_faces][3]) + bias < 0) {
                    glm::vec3 temp = faces[num_faces][0];
                    faces[num_faces][0] = faces[num_faces][1];
                    faces[num_faces][1] = temp;
                    faces[num_faces][3] = -faces[num_faces][3];
                }
                num_faces++;
            }
        } //End for iterations
        printf("EPA did not converge\n");
        //Return most recent closest point
        return faces[closest_face][3] * glm::dot(faces[closest_face][0], faces[closest_face][3]);
    }
}

namespace Player
{
    //Player data
    glm::vec3 player_pos = glm::vec3(0, 2, 0);
    glm::vec3 player_scale = glm::vec3(0.5, 0.9, 0.5);
    glm::mat4 player_M = glm::translate(glm::scale(glm::mat4(1.0f), player_scale), player_pos);
    glm::vec3 player_vel = glm::vec3(0, 0, 0);
    glm::vec4 player_colour = glm::vec4(0.1f, 0.8f, 0.3f, 1.0f);
    bool player_is_on_ground = false;
    bool player_is_jumping = false;
    float player_max_stand_slope = 45;
    //Physics stuff
    //Thanks to Kyle Pittman for his GDC talk:
    // http://www.gdcvault.com/play/1023559/Math-for-Game-Programmers-Building
    float player_top_speed = 10.0f;
    float player_time_till_top_speed = 0.25f; //Human reaction time?
    float player_acc = player_top_speed / player_time_till_top_speed;
    float friction_factor = 0.3f; //higher is slippier
    float player_jump_height = 2.0f;
    float player_jump_dist_to_peak = 2.0f; //how far on xz p moves before reaching peak jump height
    float g = -2 * player_jump_height * player_top_speed * player_top_speed / (player_jump_dist_to_peak * player_jump_dist_to_peak);
    float jump_vel = 2 * player_jump_height * player_top_speed / player_jump_dist_to_peak;

    void player_update(float dt) 
    {

        bool player_moved = false;
        //Find player's forward and right movement directions
        //glm::vec3 fwd_xz_proj = glm::normalize(glm::vec3(g_camera.fwd.x, 0, g_camera.fwd.z));
        //glm::vec3 rgt_xz_proj = glm::normalize(glm::vec3(g_camera.rgt.x, 0, g_camera.rgt.z));

        //WASD Movement (constrained to the x-z plane)
        if (IsKeyboardKeyDown(KEY_W))
        {
            player_vel += /*fwd_xz_proj **/ player_acc * dt;
            player_moved = true;
        }
        //else if (glm::dot(fwd_xz_proj, player_vel) > 0) player_vel -= fwd_xz_proj * player_acc * dt;

        if (IsKeyboardKeyDown(KEY_A))
        {
            player_vel += -/*rgt_xz_proj **/ player_acc * dt;
            player_moved = true;
        }
        //else if (dot(-rgt_xz_proj, player_vel) > 0) player_vel += rgt_xz_proj * player_acc * dt;

        if (IsKeyboardKeyDown(KEY_S)) 
        {
            player_vel += -/*fwd_xz_proj **/ player_acc * dt;
            player_moved = true;
        }
        //else if (dot(-fwd_xz_proj, player_vel) > 0) player_vel += fwd_xz_proj * player_acc * dt;

        if (IsKeyboardKeyDown(KEY_D)) 
        {
            player_vel += /*rgt_xz_proj **/ player_acc * dt;
            player_moved = true;
        }
        //else if (dot(rgt_xz_proj, player_vel) > 0) player_vel -= rgt_xz_proj * player_acc * dt;
        // NOTE about the else statements above: Checks if we aren't pressing a button 
        // but have velocity in that direction, if so slows us down faster w/ subtraction
        // This improves responsiveness and tightens up the feel of moving
        // Mult by friction_factor is good to kill speed when idle but feels drifty while moving

        if (player_is_on_ground) 
        {
            //Clamp player speed
            if (glm::length2(player_vel) > player_top_speed * player_top_speed) 
            {
                player_vel = glm::normalize(player_vel);
                player_vel *= player_top_speed;
            }
            //Deceleration
            if (!player_moved) player_vel = player_vel * friction_factor;

            static bool jump_button_was_pressed = false;
            if (IsKeyboardKeyDown(KEY_SPACE))
            {
                if (!jump_button_was_pressed) {
                    player_vel.y += jump_vel;
                    player_is_on_ground = false;
                    player_is_jumping = true;
                    jump_button_was_pressed = true;
                }
            }
            else jump_button_was_pressed = false;
        }
        else 
        { //Player is not on ground
            if (player_is_jumping) 
            {
                //If you don't hold jump you don't jump as high
                if (!IsKeyboardKeyDown(KEY_SPACE) && player_vel.y > 0) player_vel.y += 5 * g * dt;
            }

            //Clamp player's xz speed
            glm::vec3 xz_vel = glm::vec3(player_vel.x, 0, player_vel.z);
            if (glm::length(xz_vel) > player_top_speed) {
                xz_vel = glm::normalize(xz_vel);
                xz_vel *= player_top_speed;
                player_vel.x = xz_vel.x;
                player_vel.z = xz_vel.z;
            }
            player_vel.y += g * dt;
        }

        //Update player position
        player_pos += player_vel * dt;

        //Collided into ground
        if (player_pos.y < 0)
        {
            player_pos.y = 0;
            player_vel.y = 0.0f;
            player_is_on_ground = true;
            player_is_jumping = false;
        }

        if (player_pos.x < -15) player_pos.x = -15;
        if (player_pos.x > 15) player_pos.x = 15;
        if (player_pos.z < -15) player_pos.z = -15;
        if (player_pos.z > 15) player_pos.z = 15;

        //Update matrices
        player_M = glm::translate(glm::scale(glm::mat4(1.0f), player_scale), player_pos);
    }
}


g3d::Model cubeModel;
g3d::Model sphereModel;
g3d::Model cylinderModel;
g3d::Model capsuleModel;

//Set up level geometry
#define NUM_BOXES 5
glm::mat4 box_M[NUM_BOXES];
glm::vec4 box_colour[NUM_BOXES];
temp::BBox box_collider[NUM_BOXES];
#define NUM_SPHERES 3
glm::mat4 sphere_M[NUM_SPHERES];
glm::vec4 sphere_colour[NUM_SPHERES];
temp::Sphere sphere_collider[NUM_SPHERES];
#define NUM_CYLINDERS 3
glm::mat4 cylinder_M[NUM_CYLINDERS];
glm::vec4 cylinder_colour[NUM_CYLINDERS];
temp::Cylinder cylinder_collider[NUM_CYLINDERS];

const glm::vec3 box_pos[NUM_BOXES] = {
           glm::vec3(-6, 0,-6),
            glm::vec3(-6, 0, 6),
            glm::vec3(0, 0, 0),
            glm::vec3(6, 0,-6),
            glm::vec3(6, 0, 6)
};

const glm::vec3 box_scale[NUM_BOXES] = {
    glm::vec3(5.0f, 2.0f, 5.0f),
    glm::vec3(5.0f, 1.0f, 5.0f),
    glm::vec3(5.0f, 1.0f, 5.0f),
    glm::vec3(5.0f, 3.0f, 5.0f),
    glm::vec3(5.0f, 1.0f, 5.0f)
};

const glm::vec3 sphere_pos[NUM_SPHERES] = {
            glm::vec3(-6, 3,-6),
            glm::vec3(-6, 2, 6),
            glm::vec3(6, 5, -6)
};

temp::Capsule player_collider;

// const used to convert degrees into radians
#define TAU 2.0 * Pi
#define ONE_DEG_IN_RAD (2.0 * Pi) / 360.0 // 0.017444444
#define ONE_RAD_IN_DEG 360.0 / (2.0 * Pi) //57.2957795
#define DEG2RAD(a) ((a)*(Pi/180.0))
#define RAD2DEG(a) ((a)*(180.0/Pi))

//// rotate around x axis by an angle in degrees
//inline glm::mat4 rotate_x_deg(const glm::mat4& m, float deg) {
//    // convert to radians
//    float rad = deg * ONE_DEG_IN_RAD;
//    glm::mat4 m_r = glm::mat4(1.0f);
//    m_r.m[5] = cos(rad);
//    m_r.m[9] = -sin(rad);
//    m_r.m[6] = sin(rad);
//    m_r.m[10] = cos(rad);
//    return m_r * m;
//}
//
//// rotate around y axis by an angle in degrees
//inline glm::mat4 rotate_y_deg(const glm::mat4& m, float deg) {
//    // convert to radians
//    float rad = deg * ONE_DEG_IN_RAD;
//    glm::mat4 m_r = glm::mat4(1.0f);
//    m_r.m[0] = cos(rad);
//    m_r.m[8] = sin(rad);
//    m_r.m[2] = -sin(rad);
//    m_r.m[10] = cos(rad);
//    return m_r * m;
//}
//
//// rotate around z axis by an angle in degrees
//inline glm::mat4 rotate_z_deg(const glm::mat4& m, float deg) {
//    // convert to radians
//    float rad = deg * ONE_DEG_IN_RAD;
//    glm::mat4 m_r = glm::mat4(1.0f);
//    m_r.m[0] = cos(rad);
//    m_r.m[4] = -sin(rad);
//    m_r.m[1] = sin(rad);
//    m_r.m[5] = cos(rad);
//    return m_r * m;
//}

void InitTest()
{
	ncamera.Teleport(0, 5, -10);
	ncamera.LookAt(glm::vec3(0, 0, 0));

	ncamera.Enable();
	ncamera.m_speed = 1;

    //box_M[0] = glm::translate(glm::rotate((glm::scale(box_scale[0]), 45), box_pos[0]);
    box_M[0] = glm::translate(glm::scale(box_scale[0]), box_pos[0]);
    box_M[1] = glm::translate(glm::scale(box_scale[1]), box_pos[1]);
    box_M[2] = glm::translate(glm::scale(box_scale[2]), box_pos[2]);
    //box_M[3] = glm::translate(glm::rotate_x_deg(glm::scale(box_scale[3]), 40), box_pos[3]);
    //box_M[4] = glm::translate(glm::rotate_z_deg(glm::scale(box_scale[4]), 50), box_pos[4]);
    box_M[3] = glm::translate(glm::scale(box_scale[3]), box_pos[3]);
    box_M[4] = glm::translate(glm::scale(box_scale[4]), box_pos[4]);


    //Set up physics objects
    for (int i = 0; i < NUM_BOXES; i++)
    {
        box_collider[i].pos = box_pos[i];
        box_collider[i].min = glm::vec3(-0.5, 0, -0.5);
        box_collider[i].max = glm::vec3(0.5, 1, 0.5);
        box_collider[i].matRS = box_M[i];
        box_collider[i].matRS_inverse = glm::inverse(box_M[i]);
        box_colour[i] = glm::vec4(0.8f, 0.1f, 0.1f, 1);
    }

    sphere_M[0] = glm::translate(sphere_pos[0]);
    sphere_M[1] = glm::translate(sphere_pos[1]);
    sphere_M[2] = glm::translate(sphere_pos[2]);

    //Set up physics objects
    for (int i = 0; i < NUM_SPHERES; i++)
    {
        sphere_collider[i].pos = sphere_pos[i];
        sphere_collider[i].r = 1;
        sphere_collider[i].matRS = sphere_M[i];
        sphere_collider[i].matRS_inverse = inverse(sphere_M[i]);
        sphere_colour[i] = glm::vec4(0.1f, 0.8f, 0.1f, 1);
    }

    {
        const glm::vec3 cylinder_pos[NUM_CYLINDERS] = {
            glm::vec3(6, 0, 0),
            glm::vec3(-6, 0, 0),
            glm::vec3(0, 0,-6)
        };
        const float cylinder_r[NUM_CYLINDERS] = {
            2, 1, 3
        };
        const float cylinder_h[NUM_CYLINDERS] = {
            2, 3, 3
        };

        cylinder_M[0] = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(cylinder_r[0], cylinder_h[0], cylinder_r[0])), cylinder_pos[0]);
        cylinder_M[1] = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(cylinder_r[1], cylinder_h[1], cylinder_r[1])), cylinder_pos[1]);
        cylinder_M[2] = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(cylinder_r[2], cylinder_h[2], cylinder_r[2])), cylinder_pos[2]);

        //Set up physics objects
        for (int i = 0; i < NUM_CYLINDERS; i++)
        {
            cylinder_collider[i].pos = cylinder_pos[i];
            cylinder_collider[i].r = 1;
            cylinder_collider[i].y_base = cylinder_pos[i].y;
            cylinder_collider[i].y_cap = cylinder_pos[i].y + 1;
            cylinder_collider[i].matRS = cylinder_M[i];
            cylinder_collider[i].matRS_inverse = glm::inverse(cylinder_M[i]);
            cylinder_colour[i] = glm::vec4(0.8f, 0.1f, 0.8f, 1);
        }
    }

    player_collider.pos = Player::player_pos;
    player_collider.matRS = Player::player_M;
    player_collider.matRS_inverse = glm::inverse(Player::player_M);
    player_collider.r = 1;
    player_collider.y_base = 1;
    player_collider.y_cap = 2;





	// Load shader
	{
		shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
		shader.Bind();
		shader.SetUniform("uSampler", 0);
		worldUniform = shader.GetUniformVariable("uWorld");
		viewUniform = shader.GetUniformVariable("uView");
		projectionUniform = shader.GetUniformVariable("uProjection");
	}

	// Load Texture
	{
		material.diffuseTexture = TextureLoader::LoadTexture2D("../data/textures/crate.png");
	}

	// Load geometry
	{
		model.Create("../data/models/rock.obj");
		model.SetMaterial(material);

		polyModel = model.GetPoly();
	}

	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));
}

void CloseTest()
{
	shader.Destroy();
}


// Capsule pos

glm::vec3 wasdec2 = glm::vec3(-4, 0, 0);
glm::vec3 oldwasdec2 = glm::vec3(-4, 0, 0);

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

	wasdec2 += tempMath::scale3(glm::vec3(IsKeyboardKeyDown(KEY_L) - IsKeyboardKeyDown(KEY_J), IsKeyboardKeyDown(KEY_U) - IsKeyboardKeyDown(KEY_O), IsKeyboardKeyDown(KEY_I) - IsKeyboardKeyDown(KEY_K)), ncamera.m_speed * deltaTime);

	ncamera.Move(wasdec.x, wasdec.y, wasdec.z);
	ncamera.Fps(mouse.x, mouse.y);

	std::string ss = std::to_string(ncamera.m_position.x) + "|" + std::to_string(ncamera.m_position.y) + "|" + std::to_string(ncamera.m_position.z);
	puts(ss.c_str());

	shader.Bind();

	shader.SetUniform(viewUniform, ncamera.m_view);
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());
	shader.SetUniform(worldUniform, transform.GetWorld());
	model.Draw();

	//DebugDraw::DrawGrid(0);

	unsigned rgbSel;
	

	// Poly-Capsule (GJK) intersection
	{
		float x = wasdec2.x;
		float y = wasdec2.y;
		float z = wasdec2.z;
		Capsule c = Capsule(glm::vec3(x, y, z), glm::vec3(x, y + 0.5f, z), 0.2f);

		collide::GJKResult gjk;
		if (collide::PolyHitCapsule(&gjk, polyModel, c))
		{
			rgbSel = RED;
			wasdec2 = oldwasdec2;
		}			
		else
		{
			oldwasdec2 = wasdec2;
			rgbSel = GREEN;
		}

		x = wasdec2.x;
		y = wasdec2.y;
		z = wasdec2.z;
		c = Capsule(glm::vec3(x, y, z), glm::vec3(x, y + 0.5f, z), 0.2f);

		DebugDraw::DrawCapsule(c.a, c.b, c.r, rgbSel);

		DebugDraw::DrawBox(gjk.p0, glm::vec3(0.05f, 0.05f, 0.05f), WHITE);
		DebugDraw::DrawBox(gjk.p1, glm::vec3(0.05f, 0.05f, 0.05f), PURPLE);
		DebugDraw::DrawLine(gjk.p0, gjk.p1, rgbSel);

		DebugDraw::Flush(ncamera);
	}
}