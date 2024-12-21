#pragma once
#include <vector>
#include "GameObject.h"
#include <memory>
#include <GLFW/glfw3.h>
#include "MouseInteraction.h"
#include "Graphics.h"
#include <functional>
#include "MouseInteraction.h"
#include "UIButton.h"
#include "HUD.h"

struct WindowContext {
	MouseInteractionAPI* mouseAPI;
	float tileWidth;
	float tileHeight;
	// Add other APIs or data as needed
};


class World
{
private:

	MouseState mouseState;
	GridConfig gridConfig;
	GLFWwindow* Window;
	// Add panning variables
	float panX = 0.0f;
	float panY = 0.0f;
	float Zoom = 0.25f;
	//
	std::vector<std::shared_ptr<GameObject>> GameObjects;
	std::shared_ptr<class PlayerObject> Player;
	//
	std::vector<ShaderEntry> Shaders;
	Shader GridShader;
	Shader BackGround;
	//
	void InitBackground();
	void InitHUD();
	void InitGrid(const std::string& InFileName);
	void ProcessInputGL(GLFWwindow* window);
	void GarbageCollection();
	void InputUpdate();
	void SetupMouseCallbacks();
	void RenderUpdate();
	void CollisionUpdate();
	void HandleCollision(GameObject& GameObject1, GameObject& GameObject2);
public:
	std::unique_ptr<HUD> GameHUD;


	World(std::vector<std::shared_ptr<GameObject>>& GameObjects);
	World(const std::string& InFileName);
	~World();
	void Update(float DeltaSeconds);
	bool IsRunning();
};