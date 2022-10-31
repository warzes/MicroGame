#include "stdafx.h"
#include "Engine.h"
#include "LauncherApp.h"
#include <iostream>
//-----------------------------------------------------------------------------
//https://github.com/Chr157i4n/OpenGL_Game
// https://github.com/yagero/glsl-demo

// https://kingscrook.itch.io/kings-crook
// https://www.youtube.com/user/badsectoracula/videos

/*
идеи

по коду
	брать минималку с рейлиба
	но принци такой:
		- системы из глобальных функций в неймспейсе
		- объекты - классы с методами

		спрайты оружий поискать с модов майнкрафта

мин код
	https://github.com/AlexMartinelle/PicoEngine
	https://github.com/groverburger/g3d
	https://github.com/michaelh800/thinmatrix-game-engine

интерфейс похожий на это
	https://cowthing.itch.io/magic-farm

3д оформление такое
	https://cowthing.itch.io/frog-smith
	https://itey.itch.io/royal-goblin-hunt
	https://store.steampowered.com/app/1845670/Islands_of_the_Caliph/
	https://www.youtube.com/watch?v=kXcER1fi8KQ

3д стиль
	https://sketchfab.com/3d-models/forest-clearing-3223835ead1040deb9b7b2a67b277d89
	https://sketchfab.com/3d-models/pokemon-fan-art-sinnoh-battle-170e4943531847aaa6a994b509566dfb
	https://sketchfab.com/3d-models/the-legend-of-zelda-links-awakening-dx-534951458de04bfaa6548c5deee98ac6
	Legend Of Zelda: Link's Awakening

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
	https://dissident-studio.itch.io/pyramid

	https://matt-lucas.itch.io/level-buddy
	https://www.youtube.com/watch?v=55fGgHhF2DM
https://www.youtube.com/watch?v=-07InyEjtQU



просто иеди
	hexx game(1994)
*/

#if !USE_TEST
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
	vec4 textureClr = texture(uSampler, vTexCoord);
	if (textureClr.a < 0.02)
		discard;

	fragColor = textureClr;
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


VertexArrayBuffer vao;
VertexBuffer vb;
ShaderProgram shader;
UniformVariable mvpUniform;

g3d::Model model;
ShaderProgram shader2;
UniformVariable mvpUniform2;
Texture2D texture2d;

ShaderProgram shader3;


VertexBuffer instancevb;
ShaderProgram shader4;
UniformVariable viewUniform;
UniformVariable projUniform;

g3d::Camera camera;

g3d::Model modelearth;
g3d::Model modelmoon;
g3d::Model modelbackground;

Texture2D texture2dearth;
Texture2D texture2dmoon;
Texture2D texture2dbackground;

scene::Transform trans;

g3d::Model modelTest;
g3d::Model modelTest2;
#endif
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	if (engine::CreateEngine({}))
	{
#if USE_TEST
		InitTest();
#else
		SetMouseLock(true);

		//{
		//	model.Create("../data/models/crate.obj");

		shader2.CreateFromMemories(vertex_shader_text2, fragment_shader_text2);
		mvpUniform2 = shader2.GetUniformVariable("MVP");
		auto sampler = shader2.GetUniformVariable("uSampler");
		shader2.Bind();
		shader2.SetUniform(sampler, 0);


		//modelTest.Create("../data/models/test/test.obj", "../data/models/test/");
		modelTest2.Create("../data/models/map.obj", "../data/models/");

		//	Texture2DLoaderInfo info;
		//	info.fileName = "../data/textures/crate.png";
		//	texture2d.CreateFromFiles(info);

		//	shader3.CreateFromMemories(vertex_shader_text3, fragment_shader_text2);
		//	sampler = shader3.GetUniformVariable("uSampler");
		//	shader3.Bind();
		//	shader3.SetUniform(sampler, 0);
		//}

		//camera.SetSpeed(60);

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

		instancevb.Create(RenderResourceUsage::Static, amount, sizeof(glm::mat4), &modelMatrices[0]);


		const std::vector<VertexAttributeRaw> vertTnsAttr =
		{
			{.type = VertexAttributeTypeRaw::Matrix4, .normalized = false},
		};

		model.Create("../data/models/rock.obj", "../data/models/");
		model.SetInstancedBuffer(&instancevb, vertTnsAttr);
		shader4.CreateFromMemories(vertex_shader_text4, fragment_shader_text2);
		sampler = shader4.GetUniformVariable("uSampler");
		shader4.Bind();
		shader4.SetUniform(sampler, 0);
		viewUniform = shader4.GetUniformVariable("view");
		projUniform = shader4.GetUniformVariable("projection");

		Texture2DLoaderInfo info;
		info.fileName = "../data/models/rock.png";
		texture2d.CreateFromFiles(info);


		modelearth.Create("../data/models/sphere.obj");
		modelmoon.Create("../data/models/sphere.obj");
		modelbackground.Create("../data/models/sphere.obj");


		info.fileName = "../data/textures/earth.png";
		texture2dearth.CreateFromFiles(info);

		info.fileName = "../data/textures/moon.png";
		texture2dmoon.CreateFromFiles(info);

		info.fileName = "../data/textures/starfield.png";
		texture2dbackground.CreateFromFiles(info);
#endif
		while (engine::IsRunningEngine())
		{
			const float deltaTime = engine::GetDeltaTime();
			engine::BeginFrameEngine();
#if USE_TEST
			FrameTest(deltaTime);
#else
			{
				camera.SimpleMove(deltaTime);
				camera.Update();

				texture2d.Bind();
				shader2.Bind();
				trans.Reset();
				glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
				shader2.SetUniform(mvpUniform2, MVP);
				modelTest.Draw();

				modelTest2.Draw();




				texture2d.Bind();
				shader4.Bind();
				shader4.SetUniform(viewUniform, camera.GetViewMatrix());
				shader4.SetUniform(projUniform, GetCurrentProjectionMatrix());
				model.Draw();


				{
					shader2.Bind();

					trans.Reset();
					trans.SetPosition({ 4.0f, 0.0f, 0.0f });
					glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
					shader2.SetUniform(mvpUniform2, MVP);
					texture2dearth.Bind();
					modelearth.Draw();
				}

				{
					shader2.Bind();

					static float timer = 0;
					timer = timer + deltaTime;

					trans.Reset();
					trans.SetPosition({ cos(timer) * 5 + 4, sin(timer) * 5.0f, 0.0f });
					trans.SetRotate({ 0, 0, timer - glm::pi<float>() / 2.0f });
					trans.SetScale(glm::vec3(0.5f));
					glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
					shader2.SetUniform(mvpUniform2, MVP);
					texture2dmoon.Bind();
					modelmoon.Draw();
				}

				{
					shader2.Bind();

					trans.Reset();
					trans.SetScale(glm::vec3(500.0f));
					glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
					shader2.SetUniform(mvpUniform2, MVP);
					texture2dbackground.Bind();
					modelbackground.Draw();
				}

				//shader2.Bind();
				//texture2d.Bind();

				//trans.Reset();
				//glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
				//shader2.SetUniform(mvpUniform2, MVP);
				////model.Draw();

				//{
				//	trans.Reset();
				//	trans.SetPosition(glm::vec3(2.0f, 0.0f, 0.0f));
				//	trans.SetRotate(glm::vec3(glm::radians(45.0f), glm::radians(45.0f), 0.0f));
				//	MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
				//	shader2.SetUniform(mvpUniform2, MVP);
				//	//model.Draw();
				//}
				//
				//{
				//	trans.Reset();
				//	trans.SetPosition(glm::vec3(-2.0f, 1.0f, 0.0f));
				//	MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
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
				//		MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix() * trans.GetWorldMatrix();
				//		shader3.SetUniform(("MVPs[" + std::to_string(i) + "]").c_str(), MVP);
				//	}
				//	model.Draw(200);
				//}
			}
			
			if (IsKeyPressed(KEY_SPACE))
				LogPrint("space");

			auto x = GetMouseX();
			auto y = GetMouseY();

			//std::cout << x << ":" << y << std::endl;
		
#endif
			engine::EndFrameEngine();
		}
	}


	engine::DestroyEngine();
}