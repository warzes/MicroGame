#pragma once

//-----------------------------------------------------------------------------
bool StartGameApp()
{
	

	// Load Texture
	{
		material.diffuseTexture = TextureLoader::LoadTexture2D("../data/textures/tileset.png");
	}

	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));

	return true;
}

void CloseGameApp()
{
	shader.Destroy();
}

void UpdateGameApp(float deltaTime)
{
	
}

void FrameGameApp(float deltaTime)
{
}