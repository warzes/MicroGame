#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Utility.h"
#include "Camera.h"
#include "Cubemap.h"
#include "Framebuffer.h"
#include "Heightmap.h"
#include "Light.h"
#include "Model.h"
#include "Texture.h"
#include "TexturedModel.h"
#include "Entity.h"
#include "Wavefront.h"
#include "Manager.h"
#include "Shader.h"
#include "Terrain.h"
#include "Water.h"
#include "Renderer.h"

#include "BloomShader.h"
#include "BlurShader.h"
#include "DiffuseShader.h"
#include "FilterShader.h"
#include "LumaShader.h"
#include "QuadShader.h"
#include "SkyboxShader.h"
#include "TerrainShader.h"
#include "WaterShader.h"

temp::Renderer* renderer;

// Load shaders.
temp::DiffuseShader* diffuseShader;
temp::SkyboxShader* skyboxShader;
temp::TerrainShader* terrainShader;
temp::WaterShader* waterShader;
temp::QuadShader* quadShader;
temp::FilterShader* filterShader;
temp::LumaShader* lumaShader;
temp::BlurShader* blurShader;
temp::BloomShader* bloomShader;

temp::Cubemap skybox;
temp::Texture treeTexture;
temp::Texture houseTexture;
temp::Texture terrainTexture1;
temp::Texture terrainTexture2;
temp::Texture terrainTexture3;
temp::Texture waterDuDvTexture;
temp::Texture waterNormalTexture;

// Load models.
temp::Model treeModel;
temp::Model houseModel;

temp::TexturedModel* treeEntity;
temp::TexturedModel* houseEntity;

temp::TerrainObject terrainObject;
float waterLevel = 0.0f;
temp::WaterObject waterObject;
std::vector<temp::Entity> treeEntities;
std::vector<temp::Entity> houseEntities;

temp::Framebuffer postProcessingSource;
temp::Framebuffer postProcessingLuma;
temp::Framebuffer postProcessingBlurX;
temp::Framebuffer postProcessingBlurY;
temp::Framebuffer postProcessingBloom;

// Create a full-screen quad for post-processing.
temp::Model fullScreenQuad;

// Create a light.
temp::Light light(
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(1.0f, 1.0f, 1.0f)
);

// Create a camera.
temp::Camera camera(glm::vec3(0.0f, 100.0f, 100.0f), 0.0f, 0.0f, 0.0f);

// Draw state.
bool drawTrees = false;
bool drawHouses = false;
bool drawImGui = true;

// Keyboard state.
bool keyW = false;
bool keyA = false;
bool keyS = false;
bool keyD = false;
bool keyZ = false;
bool keyX = false;
bool keyLeftShift = false;
bool keyRightShift = false;

// Mouse state.
int mouseX = 0;
int mouseY = 0;
int mouseDragX = 0;
int mouseDragY = 0;
bool mouseButtonLeft = false;
bool mouseButtonMiddle = false;
bool mouseButtonRight = false;

// Cursor state.
float cursorSize = 10.0f;

Time gTime;

// Render the scene.
auto RenderScene = [&](glm::vec4 clipPlane = glm::vec4(0.0f)) 
{
	// Render the skybox.
	{
		// Enable the shader.
		skyboxShader->enable();
		skyboxShader->setProjection(renderer->projection);
		skyboxShader->setView(camera.getCubemapView());
		skyboxShader->setLight(light);

		// Render the skybox.
		renderer->renderCubemap(skybox, skyboxShader);

		// Disable the shader.
		skyboxShader->disable();
	}

	// Render the diffuse objects.
	{
		// Enable the shader.
		diffuseShader->enable();
		diffuseShader->setProjection(renderer->projection);
		diffuseShader->setView(camera.getView());
		diffuseShader->setLight(light);
		diffuseShader->setClipPlane(clipPlane);

		if( drawTrees )
		{
			// Render the trees.
			renderer->renderEntities(treeEntities, diffuseShader);
		}
		if( drawHouses )
		{
			// Render the houses.
			renderer->renderEntities(houseEntities, diffuseShader);
		}

		// Disable the shader.
		diffuseShader->disable();
	}
};

// Render the terrain.
auto RenderTerrain = [&](glm::vec4 clipPlane = glm::vec4(0.0f)) 
{
	// Enable the shader.
	terrainShader->enable();
	terrainShader->setProjection(renderer->projection);
	terrainShader->setView(camera.getView());
	terrainShader->setLight(light);
	terrainShader->setClipPlane(clipPlane);

	// Render the terrain.
	renderer->renderTerrainObject(terrainObject, terrainShader);

	// Disable the shader.
	terrainShader->disable();
};

// Render the water.
auto RenderWater = [&]() 
{
	// Enable the shader.
	waterShader->enable();
	waterShader->setProjection(renderer->projection);
	waterShader->setView(camera.getView());
	waterShader->setLight(light);
	waterShader->setCamera(camera);
	waterShader->setTime(gTime.GetTime()); // TODO: проверить

	// Render the water.
	renderer->renderWaterObject(waterObject, waterShader);

	// Disable the shader.
	waterShader->disable();
};

bool StartGameApp()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();		
	ImGui::StyleColorsClassic();// Use ImGui dark theme.

	// Setup Platform/Renderer backends
	const char* glsl_version = "#version 330";
	extern GLFWwindow* window;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	renderer = new temp::Renderer();
	diffuseShader = new temp::DiffuseShader();
	skyboxShader = new temp::SkyboxShader();
	terrainShader = new temp::TerrainShader();
	waterShader = new temp::WaterShader();
	quadShader = new temp::QuadShader();
	filterShader = new temp::FilterShader();
	lumaShader = new temp::LumaShader();
	blurShader = new temp::BlurShader();
	bloomShader = new temp::BloomShader();

	// Load the skybox.
	skybox = temp::Manager::loadCubemap({
		"../res/skybox/bluecloud_ft.jpg",
		"../res/skybox/bluecloud_bk.jpg",
		"../res/skybox/bluecloud_up.jpg",
		"../res/skybox/bluecloud_dn.jpg",
		"../res/skybox/bluecloud_rt.jpg",
		"../res/skybox/bluecloud_lf.jpg"
		});


	// Load textures.
	treeTexture = temp::Manager::loadTexture("../res/models/Tree.png");
	houseTexture = temp::Manager::loadTexture("../res/models/House.png");
	terrainTexture1 = temp::Manager::loadTexture("../res/Terrain1.png");
	terrainTexture2 = temp::Manager::loadTexture("../res/Terrain2.png");
	terrainTexture3 = temp::Manager::loadTexture("../res/Terrain3.png");
	waterDuDvTexture = temp::Manager::loadTexture("../res/WaterDuDv.png");
	waterNormalTexture = temp::Manager::loadTexture("../res/WaterNormal.png");

	// Load models.
	treeModel = temp::Manager::loadModel("../res/models/Tree.obj");
	houseModel = temp::Manager::loadModel("../res/models/House.obj");

	// Link models with their textures.
	treeEntity = new temp::TexturedModel(treeModel, treeTexture);
	houseEntity = new temp::TexturedModel(houseModel, houseTexture);

	// Create terrain.
	const float terrainSize = 500.0f;
	const int terrainResolution = 384;
	terrainObject = temp::Terrain::generateTerrain(
		terrainSize,
		terrainResolution,
		terrainTexture1,
		terrainTexture2,
		terrainTexture3
	);

	// Create water.
	waterObject = temp::Water::generateWater(
		terrainSize,
		GetRenderWidth(),
		GetRenderHeight(),
		waterDuDvTexture,
		waterNormalTexture
	);

	// // Generate trees.
	for( int i = 0; i < 1000; i++ )
	{
		int attempts = 0;
	tree:
		attempts++;
		float x = temp::signedRandomFloat() * terrainSize;
		float z = temp::signedRandomFloat() * terrainSize;
		float y = terrainObject.sample(x, z);
		if( y < waterLevel + 10.0f )
		{
			if( attempts >= 10 )
			{
				continue;
			}
			goto tree;
		}
		glm::vec3 position = glm::vec3(x, y, z);
		glm::vec3 rotation = glm::vec3(0.0f, glm::radians(temp::randomFloat() * 360.0f), 0.0f);
		glm::vec3 scale = glm::vec3(temp::randomFloat() + 1.0f);
		treeEntities.push_back(temp::Entity(*treeEntity, position, rotation, scale));
	}

	// Generate houses.
	for( int i = 0; i < 10; i++ )
	{
		int attempts = 0;
	house:
		attempts++;
		float x = temp::signedRandomFloat() * terrainSize;
		float z = temp::signedRandomFloat() * terrainSize;
		float y = terrainObject.sample(x, z);
		if( y < waterLevel + 10.0f )
		{
			if( attempts >= 10 )
			{
				continue;
			}
			goto house;
		}
		glm::vec3 position = glm::vec3(x, y, z);
		glm::vec3 rotation = glm::vec3(0.0f, glm::radians(temp::randomFloat() * 360.0f), 0.0f);
		glm::vec3 scale = glm::vec3(10.0f);
		houseEntities.push_back(temp::Entity(*houseEntity, position, rotation, scale));
	}

	// Create framebuffers for post-processing.
	postProcessingSource = temp::Manager::createFramebuffer(GetRenderWidth() / 2, GetRenderHeight() / 2, GL_NEAREST);
	postProcessingLuma = temp::Manager::createFramebuffer(GetRenderWidth() / 4, GetRenderHeight() / 4, GL_LINEAR, GL_CLAMP_TO_EDGE);
	postProcessingBlurX = temp::Manager::createFramebuffer(GetRenderWidth() / 4, GetRenderHeight() / 4, GL_LINEAR, GL_CLAMP_TO_EDGE);
	postProcessingBlurY = temp::Manager::createFramebuffer(GetRenderWidth() / 4, GetRenderHeight() / 4, GL_LINEAR, GL_CLAMP_TO_EDGE);
	postProcessingBloom = temp::Manager::createFramebuffer(GetRenderWidth() / 2, GetRenderHeight() / 2, GL_NEAREST);

	// Create a full-screen quad for post-processing.
	fullScreenQuad = temp::Manager::createQuad(-1.0f, -1.0f, 2.0f, 2.0f);

	return true;
}

void CloseGameApp()
{
	// Destroy unmanaged resources.
	diffuseShader->destroy();
	skyboxShader->destroy();
	terrainShader->destroy();
	waterShader->destroy();
	quadShader->destroy();
	filterShader->destroy();
	lumaShader->destroy();
	blurShader->destroy();
	bloomShader->destroy();

	// Clean up and exit.
	temp::Manager::cleanUp();

	delete renderer;

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void UpdateGameApp(float deltaTime)
{
	//ImGui_ImplSDL2_ProcessEvent(&e);

	if( IsKeyboardKeyUp(KEY_W) ) keyW = false;
	if( IsKeyboardKeyUp(KEY_A) ) keyA = false;
	if( IsKeyboardKeyUp(KEY_S) ) keyS = false;
	if( IsKeyboardKeyUp(KEY_D) ) keyD = false;
	if( IsKeyboardKeyUp(KEY_Z) ) keyZ = false;
	if( IsKeyboardKeyUp(KEY_X) ) keyX = false;
	if( IsKeyboardKeyUp(KEY_LEFT_SHIFT) ) keyLeftShift = false;
	if( IsKeyboardKeyUp(KEY_RIGHT_SHIFT) ) keyRightShift = false;

	if (IsKeyboardKeyDown(KEY_W) ) keyW = true;
	if( IsKeyboardKeyDown(KEY_A) ) keyA = true;
	if( IsKeyboardKeyDown(KEY_S) ) keyS = true;
	if( IsKeyboardKeyDown(KEY_D) ) keyD = true;
	if( IsKeyboardKeyDown(KEY_Z) ) keyZ = true;
	if( IsKeyboardKeyDown(KEY_X) ) keyX = true;
	if( IsKeyboardKeyDown(KEY_LEFT_SHIFT) ) keyLeftShift = true;
	if( IsKeyboardKeyDown(KEY_RIGHT_SHIFT) ) keyRightShift = true;
	if( IsKeyboardKeyDown(KEY_I) ) drawImGui = !drawImGui;

	mouseX = GetMouseX();
	mouseY = GetMouseY();
	if( mouseButtonMiddle )
	{
		mouseDragX += GetMouseDelta().x; // TODO: проверить
		mouseDragY += GetMouseDelta().y; // TODO: проверить
	}

	if( IsMouseButtonDown(0) ) mouseButtonLeft = true;
	if( IsMouseButtonDown(1) ) mouseButtonMiddle = true;
	if( IsMouseButtonDown(2) ) mouseButtonRight = true;

	if( IsMouseButtonUp(0) ) mouseButtonLeft = false;
	if( IsMouseButtonUp(1) ) mouseButtonMiddle = false;
	if( IsMouseButtonUp(2) ) mouseButtonRight = false;

	float cursorSizeDelta = 1.0f;
	if( GetMouseWheelMove() > 0 )
		cursorSize += cursorSizeDelta;
	else if( GetMouseWheelMove() < 0 )
		cursorSize -= cursorSizeDelta;


	// Point the camera.
	{
		float u = float(mouseDragX) / float(GetRenderWidth());
		float v = float(mouseDragY) / float(GetRenderHeight());
		camera.pitch = glm::radians((v * 2.0f - 1.0f) * 90.0f);
		camera.yaw = glm::radians((u * 2.0f - 1.0f) * 180.0f + 180.0f);
	}

	// Move the camera.
	{
		float cameraSpeed = 3.0f;
		if( keyLeftShift )
		{
			cameraSpeed = 10.0f;
		}
		else if( keyRightShift )
		{
			cameraSpeed = 0.3f;
		}
		if( keyW )
		{
			camera.position.x += std::sin(-camera.yaw) * -cameraSpeed;
			camera.position.z += std::cos(-camera.yaw) * -cameraSpeed;
		}
		else if( keyS )
		{
			camera.position.x += std::sin(-camera.yaw) * cameraSpeed;
			camera.position.z += std::cos(-camera.yaw) * cameraSpeed;
		}
		if( keyA )
		{
			camera.position.x += std::sin(-camera.yaw + glm::radians(90.0f)) * -cameraSpeed;
			camera.position.z += std::cos(-camera.yaw + glm::radians(90.0f)) * -cameraSpeed;
		}
		else if( keyD )
		{
			camera.position.x += std::sin(-camera.yaw + glm::radians(90.0f)) * cameraSpeed;
			camera.position.z += std::cos(-camera.yaw + glm::radians(90.0f)) * cameraSpeed;
		}
		if( keyZ )
		{
			camera.position.y += -cameraSpeed;
		}
		else if( keyX )
		{
			camera.position.y += cameraSpeed;
		}
	}


}

void FrameGameApp(float deltaTime)
{
	// Prepare the scene.
	renderer->prepare();

	// Render the scene.
	{
		// Do the reflection pass.
		{
			waterObject.bindReflectionFramebuffer();
			float D = 2.0f * (camera.position.y - waterLevel);
			camera.position.y -= D;
			camera.pitch *= -1.0f;
			RenderScene(glm::vec4(0.0f, 1.0f, 0.0f, -waterLevel));
			RenderTerrain(glm::vec4(0.0f, 1.0f, 0.0f, -waterLevel));
			camera.pitch *= -1.0f;
			camera.position.y += D;
			waterObject.unbindReflectionFramebuffer();
		}

		// Do the refraction pass.
		{
			waterObject.bindRefractionFramebuffer();
			RenderScene(glm::vec4(0.0f, -1.0f, 0.0f, waterLevel + 10.0f));
			RenderTerrain(glm::vec4(0.0f, -1.0f, 0.0f, waterLevel + 10.0f));
			waterObject.unbindRefractionFramebuffer();
		}

		// Render to the source post-processing buffer.
		postProcessingSource.bind();
		RenderScene();
		RenderTerrain();
		RenderWater();
		postProcessingSource.unbind();

		// Do the luma pass.
		lumaShader->enable();
		postProcessingLuma.bind();
		renderer->renderQuad(fullScreenQuad, postProcessingSource.textureID, lumaShader);
		postProcessingLuma.unbind();
		lumaShader->disable();

		// Do the blur pass.
		blurShader->enable();
		blurShader->modeHorizontal();
		postProcessingBlurX.bind();
		renderer->renderQuad(fullScreenQuad, postProcessingLuma.textureID, blurShader);
		postProcessingBlurX.unbind();
		blurShader->modeVertical();
		postProcessingBlurY.bind();
		renderer->renderQuad(fullScreenQuad, postProcessingBlurX.textureID, blurShader);
		postProcessingBlurY.unbind();
		blurShader->disable();

		// Do the bloom pass.
		bloomShader->enable();
		bloomShader->setTextures(postProcessingSource.textureID, postProcessingBlurY.textureID);
		postProcessingBloom.bind();
		renderer->renderUntexturedQuad(fullScreenQuad);
		postProcessingBloom.unbind();
		bloomShader->disable();

		// Do post-processing.
		filterShader->enable();
		renderer->bindDefaultFramebuffer();
		renderer->renderQuad(fullScreenQuad, postProcessingBloom.textureID, lumaShader);
		renderer->unbindDefaultFramebuffer();
		filterShader->disable();
	}

	// Render the GUI.
	if( drawImGui )
	{
		bool takeScreenshotWithImGui = false;

		renderer->prepareImGui();

		ImGui::Begin("Light");
		{
			ImGui::PushItemWidth(150);

			static float azimuth = 220.0f;
			static float altitude = 70.0f;
			ImGui::SliderFloat("Light Azimuth", &azimuth, 0.0f, 360.0f);
			ImGui::SliderFloat("Light Altitude", &altitude, 0.0f, 360.0f);

			light.direction = glm::normalize(glm::vec3(
				std::cos(glm::radians(azimuth)) * std::cos(glm::radians(altitude)),
				std::sin(glm::radians(altitude)),
				std::sin(glm::radians(azimuth)) * std::cos(glm::radians(altitude))
			));

			static float color[3] = { 1.0f, 1.0f, 1.0f };
			ImGui::ColorPicker3("Light Color", color);

			light.color = glm::vec3(
				color[0],
				color[1],
				color[2]
			);

			static float sunFalloff = 1000.0f;
			static float sunIntensity = 1.0f;
			ImGui::SliderFloat("Sun Falloff", &sunFalloff, 1.0f, 1000.0f);
			ImGui::SliderFloat("Sun Intensity", &sunIntensity, 1.0f, 100.0f);

			skyboxShader->enable();
			skyboxShader->setSun(sunFalloff, sunIntensity);
			skyboxShader->disable();

			ImGui::PopItemWidth();
		}
		ImGui::End();

		ImGui::Begin("Effects");
		{
			ImGui::PushItemWidth(200);

			// Fog settings.
			ImGui::Text("Fog Settings");
			{
				static float fogDensity = 0.009f;
				static float fogGradient = 10.0f;
				ImGui::SliderFloat("Fog Density", &fogDensity, 0.0f, 0.1f);
				ImGui::SliderFloat("Fog Gradient", &fogGradient, 0.0f, 10.0f);

				diffuseShader->enable();
				diffuseShader->setFog(fogDensity / 10.0f, fogGradient);
				diffuseShader->disable();

				terrainShader->enable();
				terrainShader->setFog(fogDensity / 10.0f, fogGradient);
				terrainShader->disable();

				waterShader->enable();
				waterShader->setFog(fogDensity / 10.0f, fogGradient);
				waterShader->disable();
			}

			// Filter settings.
			ImGui::Text("Filter Settings");
			{
				static float saturation = 0.194f;
				static float temperature = 0.011f;
				ImGui::SliderFloat("Saturation", &saturation, -1.0f, 1.0f);
				ImGui::SliderFloat("Temperature", &temperature, -0.2f, 0.2f);

				filterShader->enable();
				filterShader->setFilters(saturation, temperature);
				filterShader->disable();
			}

			// Bloom settings.
			ImGui::Text("Bloom Settings");
			{
				static float threshold = 0.55f;
				ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f);

				lumaShader->enable();
				lumaShader->setThreshold(threshold);
				lumaShader->disable();
			}

			ImGui::PopItemWidth();
		}
		ImGui::End();

		ImGui::Begin("Water");
		{
			ImGui::PushItemWidth(150);

			static float waterScale = 2.2f;
			static float waterStrength = 0.02f;
			static float waterSpeed = 0.006f;
			static float waterShinyness = 0.75f;
			static float waterSpecularity = 100.0f;
			static float waterColor[4] = { 0.0f, 0.714f, 1.0f, 0.2f };
			static float waterMurky[3] = { 0.0f, 0.026f, 0.183f };
			ImGui::SliderFloat("Water Level", &waterLevel, -50.0f, 50.0f);
			ImGui::SliderFloat("Water Scale", &waterScale, 0.0f, 100.0f);
			ImGui::SliderFloat("Water Strength", &waterStrength, 0.0f, 0.1f);
			ImGui::SliderFloat("Water Speed", &waterSpeed, 0.0f, 0.01f);
			ImGui::SliderFloat("Water Shinyness", &waterShinyness, 0.0f, 1.0f);
			ImGui::SliderFloat("Water Specularity", &waterSpecularity, 0.0f, 100.0f);
			ImGui::ColorPicker4("Water Color", waterColor);
			ImGui::SliderFloat("Water Alpha", &waterColor[3], 0.0f, 1.0f);
			ImGui::ColorPicker3("Water Murky", waterMurky);

			ImGui::PopItemWidth();

			waterShader->enable();
			waterShader->setWater(waterLevel, waterScale, waterStrength, waterSpeed, waterShinyness, waterSpecularity, glm::make_vec4(waterColor), glm::make_vec3(waterMurky));
			waterShader->disable();
		}
		ImGui::End();

		ImGui::Begin("Debug");
		{
			ImGui::Text("Camera X: %f", camera.position.x);
			ImGui::Text("Camera Y: %f", camera.position.y);
			ImGui::Text("Camera Z: %f", camera.position.z);
			ImGui::Text("Camera Pitch: %f", glm::degrees(camera.pitch));
			ImGui::Text("Camera Yaw: %f", glm::degrees(camera.yaw));
			ImGui::Text("Camera Roll: %f", glm::degrees(camera.roll));

			if( ImGui::Button("Go To Origin") )
			{
				camera.position.x = 0.0f;
				camera.position.z = 0.0f;
				mouseDragX = 0.0f;
				mouseDragY = GetRenderHeight();
			}

			if( ImGui::Button("Take Screenshot") )
			{
				//unsigned char* data = new unsigned char[renderer.width * renderer.height * 3];
				//glReadPixels(0, 0, renderer.width, renderer.height, GL_RGB, GL_UNSIGNED_BYTE, data);
				//stbi_flip_vertically_on_write(true);
				//stbi_write_png(formatString("screenshot_%d.png", time(NULL)), renderer.width, renderer.height, 3, data, renderer.width * 3);
				//delete[] data;
			}

			if( ImGui::Button("Take Screenshot With ImGui") )
			{
				takeScreenshotWithImGui = true;
			}

			if( ImGui::Button("Regenerate Terrain") )
			{
				terrainObject.heightmap.regenerate();
			}

			ImGui::Checkbox("Draw Trees", &drawTrees);
			ImGui::Checkbox("Draw Houses", &drawHouses);
		}
		ImGui::End();

		ImGui::Begin("Editor");
		{
			ImGui::PushItemWidth(150);

			static const char* modes[] = {
				"Sculpt",
				"Average"
			};
			static int mode = 0;
			ImGui::Combo("Mode", &mode, modes, IM_ARRAYSIZE(modes));

			static float power = 0.5f;
			ImGui::SliderFloat("Power", &power, 0.0f, 1.0f);

			ImGui::PopItemWidth();

			glm::vec3 mouseRay = renderer->getMouseRay(camera, mouseX, mouseY);
			float timeOfIntersection = terrainObject.raycast(camera.position, mouseRay);
			if( timeOfIntersection > 0.0f )
			{
				glm::vec3 pointOfIntersection = camera.position + mouseRay * timeOfIntersection;
				terrainShader->enable();
				terrainShader->setCursor(glm::vec2(pointOfIntersection.x, pointOfIntersection.z), cursorSize);
				terrainShader->disable();

				if( !ImGui::GetIO().WantCaptureMouse )
				{
					if( mode == 0 )
					{
						if( mouseButtonLeft )
						{
							terrainObject.addHeight(pointOfIntersection.x, pointOfIntersection.z, cursorSize, power);
							terrainObject.update();
						}
						else if( mouseButtonRight )
						{
							terrainObject.addHeight(pointOfIntersection.x, pointOfIntersection.z, cursorSize, -power);
							terrainObject.update();
						}
					}
					else if( mode == 1 )
					{
						if( mouseButtonLeft )
						{
							terrainObject.averageHeight(pointOfIntersection.x, pointOfIntersection.z, cursorSize, power);
							terrainObject.update();
						}
					}
				}

				for( auto& entity : treeEntities )
				{
					float y = terrainObject.sample(entity.position.x, entity.position.z);
					if( y < waterLevel + 10.0f )
					{
						y = -10000.0f;
					}
					entity.position.y = y;
				}
				for( auto& entity : houseEntities )
				{
					float y = terrainObject.sample(entity.position.x, entity.position.z);
					if( y < waterLevel + 10.0f )
					{
						y = -10000.0f;
					}
					entity.position.y = y;
				}
			}
		}
		ImGui::End();

		renderer->renderImGui();

		//if( takeScreenshotWithImGui )
		//{
		//	unsigned char* data = new unsigned char[renderer.width * renderer.height * 3];
		//	glReadPixels(0, 0, renderer.width, renderer.height, GL_RGB, GL_UNSIGNED_BYTE, data);
		//	stbi_flip_vertically_on_write(true);
		//	stbi_write_png(formatString("screenshot_%d.png", time(NULL)), renderer.width, renderer.height, 3, data, renderer.width * 3);
		//	delete[] data;
		//}
	}

	//texture2d.Bind(0);
	//shader.Bind();
	//vao.Draw();

	//// Start the Dear ImGui frame
	//ImGui_ImplOpenGL3_NewFrame();
	//ImGui_ImplGlfw_NewFrame();
	//ImGui::NewFrame();

	//// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	//if( show_demo_window )
	//	ImGui::ShowDemoWindow(&show_demo_window);

	//// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	//{
	//	static float f = 0.0f;
	//	static int counter = 0;

	//	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

	//	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	//	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
	//	ImGui::Checkbox("Another Window", &show_another_window);

	//	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	//	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	//	if( ImGui::Button("Button") )                            // Buttons return true when clicked (most widgets return true when edited/activated)
	//		counter++;
	//	ImGui::SameLine();
	//	ImGui::Text("counter = %d", counter);

	//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//	ImGui::End();
	//}

	//// Rendering
	//ImGui::Render();
	//int display_w, display_h;
	////glfwGetFramebufferSize(window, &display_w, &display_h);
	//display_w = GetRenderWidth();
	//display_h = GetRenderHeight();
	//glViewport(0, 0, display_w, display_h);
	//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	//glClear(GL_COLOR_BUFFER_BIT);
	//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}