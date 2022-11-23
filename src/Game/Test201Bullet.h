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
g3d::Model capsule;
g3d::Material material;
//g3d::FreeCamera camera;
Transform transform;

glm::vec3 CapsulePos = { 1.5f, 0.0f, 0.0f };

Camera ncamera;

Texture2D tex;

PhysicsObject* phGroundObj;
PhysicsObject* phRedBoxObj;
PhysicsObject* phSecondBoxObj;

void InitTest()
{
	//SetMouseLock(true);
	//camera.SetPosition({ 0,0,-20 });
	//camera.SetLook(glm::vec3(0));

	ncamera.Teleport(0, 4, -15);
	ncamera.LookAt(glm::vec3(0, 4, 0));
	ncamera.Enable();
	ncamera.m_speed = 3;


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
		model.Create("../data/models/crate.obj");
		model.SetMaterial(material);

		capsule.Create("../data/models/capsule.obj");
		capsule.SetMaterial(material);
	}	

	// create physics
	{
		// create a ground plane
		phGroundObj = PhysicsSystem::CreatePhysicsObject(new btBoxShape(btVector3(0.5, 50, 50)), 0, btVector3(0.0f, -0.5f, 0.0f));

		// create our original red box
		phRedBoxObj = PhysicsSystem::CreatePhysicsObject(new btBoxShape(btVector3(0.5, 0.5, 0.5)), 1.0, btVector3(0.0f, 10.0f, 0.0f));

		// create a second box
		phSecondBoxObj = PhysicsSystem::CreatePhysicsObject(new btBoxShape(btVector3(0.5, 0.5, 0.5)), 1.0, btVector3(0.25f, 20.0f, 0.0f));
	}
}

void CloseTest()
{
	shader.Destroy();
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

	//camera.SimpleMove(deltaTime);
	//camera.Update();

	//std::string ss = std::to_string(camera.GetPosition().x) + "|" + std::to_string(camera.GetPosition().y) + "|" + std::to_string(camera.GetPosition().z);
	//puts(ss.c_str());

	shader.Bind();

	shader.SetUniform(viewUniform, ncamera.m_view);
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());

	shader.SetUniform(worldUniform, transform.GetWorld());
	//capsule.Draw();


	// create an array of 16 floats (representing a 4x4 matrix)
	btScalar rawTransform[16];
	btCollisionShape* pShape;

	//{
	//	phGroundObj->GetTransform(transform);
	//	pShape = phGroundObj->GetShape();
	//	color = phGroundObj->GetColor();
	//	// make a different draw call based on the object type
	//	switch (pShape->getShapeType()) 
	//	{
	//		// an internal enum used by Bullet for boxes
	//	case BOX_SHAPE_PROXYTYPE:
	//	{
	//		// assume the shape is a box, and typecast it
	//		const btBoxShape* box = static_cast<const btBoxShape*>(pShape);
	//		// get the 'halfSize' of the box
	//		btVector3 halfSize = box->getHalfExtentsWithMargin();
	//		// draw the box
	//		model.Draw();
	//		break;
	//	}
	//	default:
	//		// unsupported type
	//		break;
	//	}
	//}

	{
		phRedBoxObj->GetTransform(rawTransform);
		pShape = phRedBoxObj->GetShape();

		shader.SetUniform(worldUniform, glm::make_mat4(rawTransform));

		// make a different draw call based on the object type
		switch (pShape->getShapeType())
		{
			// an internal enum used by Bullet for boxes
		case BOX_SHAPE_PROXYTYPE:
		{
			// assume the shape is a box, and typecast it
			const btBoxShape* box = static_cast<const btBoxShape*>(pShape);
			// get the 'halfSize' of the box
			btVector3 halfSize = box->getHalfExtentsWithMargin();
			// draw the box
			model.Draw();
			break;
		}
		default:
			// unsupported type
			break;
		}
	}

	{
		phSecondBoxObj->GetTransform(rawTransform);
		pShape = phSecondBoxObj->GetShape();

		shader.SetUniform(worldUniform, glm::make_mat4(rawTransform));

		// make a different draw call based on the object type
		switch (pShape->getShapeType())
		{
			// an internal enum used by Bullet for boxes
		case BOX_SHAPE_PROXYTYPE:
		{
			// assume the shape is a box, and typecast it
			const btBoxShape* box = static_cast<const btBoxShape*>(pShape);
			// get the 'halfSize' of the box
			btVector3 halfSize = box->getHalfExtentsWithMargin();
			// draw the box
			model.Draw();
			break;
		}
		default:
			// unsupported type
			break;
		}
	}


	DebugDraw::DrawGrid(0);
	DebugDraw::DrawCapsule(glm::vec3(CapsulePos.x, CapsulePos.y+0.2, CapsulePos.z), glm::vec3(CapsulePos.x, CapsulePos.y+1, CapsulePos.z), 0.2f, RED);
	DebugDraw::Flush(ncamera);
}