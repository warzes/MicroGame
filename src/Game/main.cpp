﻿#include "Engine.h"
#include <iostream>
//-----------------------------------------------------------------------------
//https://github.com/Chr157i4n/OpenGL_Game
// https://github.com/yagero/glsl-demo

// https://kingscrook.itch.io/kings-crook
// https://www.youtube.com/user/badsectoracula/videos



/*
идеи

по коду
	glfw
	брать минималку с рейлиба
	но принци такой:
		- системы из глобальных функций в неймспейсе
		- объекты - классы с методами



мин код
	https://github.com/AlexMartinelle/PicoEngine
	https://github.com/groverburger/g3d

интерфейс похожий на это
	https://cowthing.itch.io/magic-farm

3д оформление такое
	https://cowthing.itch.io/frog-smith
	https://itey.itch.io/royal-goblin-hunt
	https://store.steampowered.com/app/1845670/Islands_of_the_Caliph/
	https://www.youtube.com/watch?v=kXcER1fi8KQ

другое
	https://cenullum.itch.io/kingway
	https://baku.itch.io/legend-of-xenia-3d
	https://apapappa.itch.io/yaw
	https://iwilliams.itch.io/fiends-isle-demo
	https://voxelvoid.itch.io/dark-lords-maze
	https://t19games.itch.io/hellhuntgb
	https://malec2b.itch.io/and-all-would-cry-beware
	https://github.com/aoblet/luminolGL/
	https://github.com/r-lyeh/FWK
	https://www.youtube.com/channel/UCNz9gHQeGOC26GVwPQzr9EQ/videos


	https://matt-lucas.itch.io/level-buddy
*/


static const renderer::Vertex_Pos2_Color vertices[3] =
{
	{ {-0.6f, -0.4f}, {1.f, 0.f, 0.f} },
	{ { 0.6f, -0.4f}, {0.f, 1.f, 0.f} },
	{ {  0.f,  0.6f}, {0.f, 0.f, 1.f} }
};

static const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec3 vCol;

uniform mat4 MVP;

out vec3 color;

void main()
{
	gl_Position = MVP * vec4(vPos, 0.0, 1.0);
	color = vCol;
}
)";

static const char* fragment_shader_text = R"(
#version 330 core

in vec3 color;

out vec4 fragColor;

void main()
{
	fragColor = vec4(color, 1.0);
}
)";

static const char* vertex_shader_text2 = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 MVP;

out vec2 vTexCoord;

void main()
{
	gl_Position = MVP * vec4(aPos, 1.0);
	vTexCoord = aTexCoord;
}
)";

static const char* fragment_shader_text2 = R"(
#version 330 core

uniform sampler2D uSampler;

in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
	fragColor = texture(uSampler, vTexCoord);
}
)";

static const char* vertex_shader_text3 = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 MVPs[200];

out vec2 vTexCoord;

void main()
{
	gl_Position = MVPs[gl_InstanceID] * vec4(aPos, 1.0);
	vTexCoord = aTexCoord;
}
)";

static const char* vertex_shader_text4 = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in mat4 instanceMatrix;

uniform mat4 projection;
uniform mat4 view;

out vec2 vTexCoord;

void main()
{
	gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
	vTexCoord = aTexCoord;
}
)";


renderer::VertexArrayBuffer vao;
renderer::VertexBuffer vb;
renderer::ShaderProgram shader;
renderer::UniformVariable mvpUniform;

g3d::Model model;
renderer::ShaderProgram shader2;
renderer::UniformVariable mvpUniform2;
renderer::Texture2D texture2d;

renderer::ShaderProgram shader3;


renderer::VertexBuffer instancevb;
renderer::ShaderProgram shader4;
renderer::UniformVariable viewUniform;
renderer::UniformVariable projUniform;

#include "tiny_obj_loader.h"
#include <unordered_map>

g3d::Camera camera;

scene::Transform trans;

int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	if (engine::CreateEngine({}))
	{
		//vb.Create(renderer::RenderResourceUsage::Static, 3, sizeof(vertices[0]), vertices);
		//vao.Create(&vb, nullptr, renderer::GetVertexAttributes<renderer::Vertex_Pos2_Color>());

		//shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
		//mvpUniform = shader.GetUniformVariable("MVP");

		//{
		//	model.Create("../data/models/crate.obj");

		//	shader2.CreateFromMemories(vertex_shader_text2, fragment_shader_text2);
		//	mvpUniform2 = shader2.GetUniformVariable("MVP");
		//	auto sampler = shader2.GetUniformVariable("uSampler");
		//	shader2.Bind();
		//	shader2.SetUniform(sampler, 0);

		//	renderer::Texture2DLoaderInfo info;
		//	info.fileName = "../data/textures/crate.png";
		//	texture2d.CreateFromFiles(info);

		//	shader3.CreateFromMemories(vertex_shader_text3, fragment_shader_text2);
		//	sampler = shader3.GetUniformVariable("uSampler");
		//	shader3.Bind();
		//	shader3.SetUniform(sampler, 0);
		//}

		camera.SetSpeed(100);

		unsigned int amount = 100000;
		glm::mat4* modelMatrices;
		modelMatrices = new glm::mat4[amount];
		srand(static_cast<unsigned int>(100)); // initialize random seed
		float radius = 150.0;
		float offset = 25.0f;
		for (unsigned int i = 0; i < amount; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			// 1. translation: displace along circle with 'radius' in range [-offset, offset]
			float angle = (float)i / (float)amount * 360.0f;
			float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float x = sin(angle) * radius + displacement;
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float z = cos(angle) * radius + displacement;
			model = glm::translate(model, glm::vec3(x, y, z));

			// 2. scale: Scale between 0.05 and 0.25f
			float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
			model = glm::scale(model, glm::vec3(scale));

			// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
			float rotAngle = static_cast<float>((rand() % 360));
			model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

			// 4. now add to list of matrices
			modelMatrices[i] = model;
		}

		instancevb.Create(renderer::RenderResourceUsage::Static, amount, sizeof(glm::mat4), &modelMatrices[0]);


		const std::vector<renderer::VertexAttribute> vertTnsAttr =
		{
			{.type = renderer::VertexAttributeType::Matrix4, .normalized = false},
		};

		model.Create("../data/models/rock.obj", "../data/models/");
		model.SetInstancedBuffer(&instancevb, vertTnsAttr);
		shader4.CreateFromMemories(vertex_shader_text4, fragment_shader_text2);
		auto sampler = shader4.GetUniformVariable("uSampler");
		shader4.Bind();
		shader4.SetUniform(sampler, 0);
		viewUniform = shader4.GetUniformVariable("view");
		projUniform = shader4.GetUniformVariable("projection");

		renderer::Texture2DLoaderInfo info;
		info.fileName = "../data/models/rock.png";
		texture2d.CreateFromFiles(info);

		while (engine::IsRunningEngine())
		{
			const float deltaTime = engine::GetDeltaTime();
			engine::BeginFrameEngine();
			{
			//	float ratio = platform::GetFrameBufferAspectRatio();
			//	glm::mat4 mvp = glm::mat4(1.0f);
			////	mvp = glm::rotate(mvp, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
			//	mvp = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f) * mvp;
			//	shader.Bind();
			//	shader.SetUniform(mvpUniform, mvp);
			//	vao.Draw();

				camera.SimpleMove(deltaTime);
				camera.Update();

				texture2d.Bind();
				shader4.Bind();
				shader4.SetUniform(viewUniform, camera.GetViewMatrix());
				shader4.SetUniform(projUniform, renderer::GetCurrentProjectionMatrix());

				model.Draw();

				//shader2.Bind();
				//texture2d.Bind();

				//trans.Reset();
				//glm::mat4 MVP = renderer::GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
				//shader2.SetUniform(mvpUniform2, MVP);
				////model.Draw();

				//{
				//	trans.Reset();
				//	trans.SetPosition(glm::vec3(2.0f, 0.0f, 0.0f));
				//	trans.SetRotate(glm::vec3(glm::radians(45.0f), glm::radians(45.0f), 0.0f));
				//	MVP = renderer::GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
				//	shader2.SetUniform(mvpUniform2, MVP);
				//	//model.Draw();
				//}
				//
				//{
				//	trans.Reset();
				//	trans.SetPosition(glm::vec3(-2.0f, 1.0f, 0.0f));
				//	MVP = renderer::GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
				//	shader2.SetUniform(mvpUniform2, MVP);
				//	//model.Draw();
				//}
				//g3d::drawPrimitive::DrawCubeWires(camera, glm::vec3(0, 0, 0));

				//// instance example
				//{
				//	
				//	shader3.Bind();
				//	texture2d.Bind();
				//	srand(100);
				//	for (unsigned int i = 0; i < 200; i++)
				//	{
				//		
				//		int x = rand() % 50 - 25;
				//		int y = rand() % 10 - 5;
				//		int z = rand() % 50 - 25;

				//		trans.Reset();
				//		trans.SetPosition(glm::vec3(x, y, z));
				//		MVP = renderer::GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
				//		shader3.SetUniform(("MVPs[" + std::to_string(i) + "]").c_str(), MVP);
				//	}
				//	model.Draw(200);
				//}
			}
			engine::EndFrameEngine();

			if (platform::IsKeyPressed(platform::KEY_SPACE))
				core::LogPrint("space");

			auto x = platform::GetMouseX();
			auto y = platform::GetMouseY();

			//std::cout << x << ":" << y << std::endl;
		}
	}
	engine::DestroyEngine();
}