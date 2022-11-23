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
Transform transform;

glm::vec3 CapsulePos = { 2, 3.0f, 0.0f };

Camera ncamera;

Texture2D tex;

PhysicsObject* phGroundObj;
PhysicsObject* phRedBoxObj;
PhysicsObject* phSecondBoxObj;

Rigidbody rigidBody;
Collider capsuleCollider;
CharacterController character;

void InitTest()
{

	ncamera.Teleport(0, 4, -15);
	ncamera.LookAt(glm::vec3(0, 4, 0));
	ncamera.Enable();
	ncamera.m_speed = 3;

	// create character
	{
		capsuleCollider.CreateCapsule(0.1, 1);
		rigidBody.Set(CapsulePos, { 1.0f, 0.0f, 0.0f, 0.0f }, capsuleCollider, 1.0f);
		character.Create(&rigidBody, 6.0f, 5.0f, 1.0f);
		PhysicsSystem::Add(&rigidBody);
	}

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
		phGroundObj = PhysicsSystem::CreatePhysicsObject(new btBoxShape(btVector3(0.5, 100, 100)), 0, btVector3(0.0f, -0.5f, 0.0f));

		//// create our original red box
		//phRedBoxObj = PhysicsSystem::CreatePhysicsObject(new btBoxShape(btVector3(0.5, 0.5, 0.5)), 1.0, btVector3(0.0f, 10.0f, 0.0f));

		//// create a second box
		//phSecondBoxObj = PhysicsSystem::CreatePhysicsObject(new btBoxShape(btVector3(0.5, 0.5, 0.5)), 1.0, btVector3(0.25f, 20.0f, 0.0f));
	}
}

void CloseTest()
{
	shader.Destroy();
}

void FrameTest(float deltaTime)
{
	// camera move
	{
		bool mouseLock = IsMouseButtonDown(0);
		SetMouseLock(mouseLock);

		const float xpos = GetMouseX();
		const float ypos = GetMouseY();
		static float lastPosX = xpos;
		static float lastPosY = ypos;
		glm::vec2 mouse = tempMath::scale2(glm::vec2((lastPosX - xpos), (lastPosY - ypos)), 200.0f * deltaTime * mouseLock);
		lastPosX = xpos;
		lastPosY = ypos;
		glm::vec3 wasdec = tempMath::scale3(glm::vec3(IsKeyboardKeyDown(KEY_A) - IsKeyboardKeyDown(KEY_D), IsKeyboardKeyDown(KEY_E) - IsKeyboardKeyDown(KEY_C), IsKeyboardKeyDown(KEY_W) - IsKeyboardKeyDown(KEY_S)), ncamera.m_speed * deltaTime);

		ncamera.Move(wasdec.x, wasdec.y, wasdec.z);
		ncamera.Fps(mouse.x, mouse.y);
	}
	
	character.Update(deltaTime);




	shader.Bind();

	shader.SetUniform(viewUniform, ncamera.m_view);
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());

	/*btScalar rawTransform[16];
	btCollisionShape* pShape;
	{
		phRedBoxObj->GetTransform(rawTransform);
		pShape = phRedBoxObj->GetShape();

		shader.SetUniform(worldUniform, glm::make_mat4(rawTransform));

		switch (pShape->getShapeType())
		{
		case BOX_SHAPE_PROXYTYPE:
		{
			const btBoxShape* box = static_cast<const btBoxShape*>(pShape);
			btVector3 halfSize = box->getHalfExtentsWithMargin();
			model.Draw();
			break;
		}
		}
	}
	{
		phSecondBoxObj->GetTransform(rawTransform);
		pShape = phSecondBoxObj->GetShape();

		shader.SetUniform(worldUniform, glm::make_mat4(rawTransform));

		switch (pShape->getShapeType())
		{
		case BOX_SHAPE_PROXYTYPE:
		{
			const btBoxShape* box = static_cast<const btBoxShape*>(pShape);
			btVector3 halfSize = box->getHalfExtentsWithMargin();
			model.Draw();
			break;
		}
		}
	}*/


	DebugDraw::DrawGrid(0);

	glm::vec3 tcapsulePos;
	glm::quat tcapsuleRot;
	rigidBody.GetPosAndRot(tcapsulePos, tcapsuleRot);
	DebugDraw::DrawCapsule(glm::vec3(tcapsulePos.x, tcapsulePos.y, tcapsulePos.z), glm::vec3(tcapsulePos.x, tcapsulePos.y+1, tcapsulePos.z), 0.2f, RED);

	DebugDraw::Flush(ncamera);
}