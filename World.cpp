#include "World.h"
#include "Graphics.h"
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "PlayerObject.h"
#include "Math.h"
#include "XMLParser.h"
#include <algorithm>
#include "UIPaginatedWindow.h"


World::World(std::vector<std::shared_ptr<GameObject>>& GameObjects)
{

}

World::World(const std::string& InFileName)
{

	StartUpData Temp = XMLParser::LoadLeveL(InFileName);
	Window = Graphics::InitWindow(Temp.LevelWidth * Temp.WindowScale, Temp.LevelHeight * Temp.WindowScale);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	InitBackground();
	InitGrid(Temp.GridFileName);
	SetupMouseCallbacks();


	std::shared_ptr<UIPaginatedWindow> shopWindow = std::make_shared<UIPaginatedWindow>(0.0f, 0.0f, 1.5f, 1.5f);
	Graphics::DrawUIElement(*shopWindow, "grass.png");

	std::shared_ptr<UIButton> nextButton = std::make_shared<UIButton>(0.6f, -0.7f, 0.2f, 0.1f, [&]() {
		shopWindow->nextPage();

		});
	Graphics::DrawUIElement(*nextButton, "shop.png");

	std::shared_ptr<UIButton> prevButton = std::make_shared<UIButton>(-0.6f, -0.7f, 0.2f, 0.1f, [&]() {
		shopWindow->previousPage();
		});
	Graphics::DrawUIElement(*prevButton, "shop.png");

	shopWindow->pageControls.push_back(nextButton);
	shopWindow->pageControls.push_back(prevButton);
	shopWindow->SetHidden(true);


	std::weak_ptr<UIElement> weakSdd = shopWindow; // Create a weak pointer

	std::shared_ptr<UIButton> sd = std::make_shared<UIButton>(0.80f, -0.82f, .3f, .3f, [weakSdd]() {
		std::cout << "Shop button clicked! Opening Shop UI..." << std::endl;
		if (auto sharedSdd = weakSdd.lock()) { // Check if the weak pointer is still valid
			std::cout << "Toggling Shop Button visibility!" << std::endl;
			sharedSdd->SetHidden(!sharedSdd->isHidden);
		}
		else {
			std::cerr << "Shop button is no longer valid!" << std::endl;
		}
		});
	Graphics::DrawUIElement(*sd,"shop.png");
	uis.push_back(sd);
	uis.push_back(shopWindow);

}

World::~World()
{
	for (auto it = Shaders.begin(); it < Shaders.end(); it++)
	{
		glDeleteVertexArrays(1, &it->shader.VAO);
		glDeleteBuffers(1, &it->shader.VBO);
		glDeleteBuffers(1, &it->shader.EBO);
	}
	MouseInteractionAPI* api = static_cast<MouseInteractionAPI*>(glfwGetWindowUserPointer(Window));
	if (api) {
		delete api; // Free the dynamically allocated memory
	}
	GameObjects.clear();
	Player = nullptr;
	Shaders.clear();
	glfwTerminate();
}

void World::InitBackground()
{
	Shaders.push_back({Graphics::DrawTexture("Map1.jpg"), ShaderType::Background });
}

void World::InitGrid(const std::string& InFileName)
{
	int windowWidth, windowHeight;
	glfwGetWindowSize(Window, &windowWidth, &windowHeight);
	gridConfig = XMLParser::ParseGridDataFromXML(InFileName);
	Shaders.push_back({ Graphics::DrawGrid(gridConfig,windowWidth,windowHeight), ShaderType::Grid });
}

void World::onHoverFunction(int gridX, int gridY, float screenX, float screenY)
{

	std::cout << "Hovereddead over tile: (" << gridX << ", " << gridY << ")" << std::endl;
	for (auto& element : uis)
	{
		if (auto button = std::dynamic_pointer_cast<UIButton>(element)) {
			// Successfully cast, so the object is a Button
			button->updateHoverState(screenX, screenY);
		}
		if (auto Window = std::dynamic_pointer_cast<UIPaginatedWindow>(element)) {
			// Successfully cast, so the object is a Button
			Window->UpdateChildrenButtons(screenX, screenY);
		}
	}
	//mouseState.GridX = gridX;
	//mouseState.GridY = gridY;
}

void World::onClickFunction(int gridX, int gridY, float screenX, float screenY)
{
	for (auto& elements : uis)
	{
		if (auto button = std::dynamic_pointer_cast<UIButton>(elements)) {
			if (button->isHovered) {
				button->cllicked();
				button->onClick();
			}
		}

	}
}


void World::ProcessInputGL(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	int windowWidth, windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	float panSpeed = 0.01f; // Speed of panning


	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		panX -= panSpeed;

	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		panX += panSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		panY += panSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		panY -= panSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		if(Zoom < 3.5f)
		Zoom +=  0.05f;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		Zoom -= 0.05f;
	}

	Zoom = std::clamp(Zoom, windowWidth/2000.0f, (windowWidth / 2000.0f)* 3.5f);
	// Calculate visible bounds based on zoom
	float halfVisibleWidth = (windowWidth / Zoom) / 2.0f;
	float halfVisibleHeight = (windowHeight / Zoom) / 2.0f;

	// Background dimensions (2000x1404 assumed)
	float bgHalfWidth = 2000.0f / 2.0f;
	float bgHalfHeight = 1404.0f / 2.0f;

	// Calculate panning bounds
	float minPanX = (- bgHalfWidth + halfVisibleWidth)/1000.0f;
	float maxPanX = (bgHalfWidth - halfVisibleWidth) / 1000.0f;

	float minPanY = (- bgHalfHeight + halfVisibleHeight)/1000.0f;
	float maxPanY = (bgHalfHeight - halfVisibleHeight)/1000.0f;
	panX = std::clamp(panX, minPanX, maxPanX);
	// 2000/ 14004 = 1.42f
	panY = std::clamp(panY, minPanY*1.42f,maxPanY*1.42f);
	MouseInteractionAPI* api = static_cast<MouseInteractionAPI*>(glfwGetWindowUserPointer(window));
	if (api) {
		api->SetPanZoom(panX, panY, Zoom);
	}
	//std::cout << "zoom: " << Zoom << std::endl;
	//std::cout << "pan: " << panX << std::endl;
	//std::cout << "panY lim: " << minPanY << ": " << maxPanY << std::endl;
}

void World::GarbageCollection()
{
	for (auto it = GameObjects.begin(); it != GameObjects.end(); )
	{
		if ((*it)->MarkedForDelete)
		{
			it = GameObjects.erase(it);
		}
		else
			++it;
	}
}

void World::InputUpdate()
{
	ProcessInputGL(Window);
}

void World::SetupMouseCallbacks()
{
	// Mouse Interaction API setup
	MouseInteractionAPI* mouseAPI = new MouseInteractionAPI(Window,gridConfig);
	mouseAPI->SetHoverCallback([this](int gridX, int gridY, float screenX, float screenY) {
		this->onHoverFunction(gridX, gridY, screenX, screenY);
		});
	mouseAPI->SetClickCallback([this](int gridX, int gridY, float screenX, float screenY) {
		this->onClickFunction(gridX, gridY, screenX, screenY);
		});
	// Set the user pointer
}

void World::RenderUpdate()
{
	//
	glClearColor(0.2f, 0.3f, 0.76f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	int windowWidth, windowHeight;
	glfwGetWindowSize(Window, &windowWidth, &windowHeight);
	// static stuff , right now only BG
	for (auto it = Shaders.begin(); it < Shaders.end(); it++)
	{
		float scaleX = 2000.0f / windowWidth;
		float scaleY = 1404.0f / windowHeight;
		glm::mat4 transform = glm::mat4(1.0f);
		//std::cout << "scale: " << scaleX << std::endl;

		// bind Texture
		// render container
		GLuint  transformLoc;
		switch (it->type)
		{
		case ShaderType::Background:

			// bind Texture
			glBindTexture(GL_TEXTURE_2D, it->shader.Texture);
			// render container
			it->shader.use();
			//it->shader.setUniform3f("panOffset", panX, panY,0.f);
			// Scale and translate the background

			transform = glm::scale(transform, glm::vec3(scaleX*Zoom, scaleY*Zoom, 1.0f));
			transform = glm::translate(transform, glm::vec3(panX, panY, 0.0f));

			  transformLoc = glGetUniformLocation((it)->shader.ID, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
			//it->shader.setFloat("zoom", Zoom);
			glBindVertexArray(it->shader.VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			break;
		case ShaderType::Grid:
			it->shader.use();
			it->shader.setUniform2i("tileCoor", mouseState.GridX, mouseState.GridY);

			transform = glm::scale(transform, glm::vec3(scaleX * Zoom, scaleY * Zoom, 1.0f));
			transform = glm::translate(transform, glm::vec3(panX, panY, 0.0f));

			transformLoc = glGetUniformLocation((it)->shader.ID, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

			glBindVertexArray(it->shader.VAO);
			//// todoooo donmt forgetteettete it shoudl be grid vertices size
			glDrawArrays(GL_LINES, 0, 200 / 2);
			break;
		}
	}

	// position update
	for (auto it = GameObjects.begin(); it != GameObjects.end(); ++it)
	{
		// create transformations
		glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		transform = glm::translate(transform, glm::vec3((*it)->GetTransform().Location[0], (*it)->GetTransform().Location[1], 0.0f));
		transform = glm::rotate(transform, (*it)->GetTransform().Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		(*it)->ObjectShader->use();
		GLuint   transformLoc = glGetUniformLocation((*it)->ObjectShader->ID, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		glBindVertexArray((*it)->ObjectShader->VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	for (auto it = uis.begin(); it != uis.end(); ++it)
	{
		(*it)->draw();
	}
	glfwSwapBuffers(Window);

}

void World::CollisionUpdate()
{

	for (auto it = GameObjects.begin(); it != GameObjects.end(); ++it)
	{
		if (!Player || Player->MarkedForDelete)
		{
			break;
		}
		if (*it == Player)continue;
		std::vector<float> Raw;
		for (size_t i = 0; i < Player->GetTransform().Location.size(); i++)
		{
			Raw.push_back((*it)->GetTransform().Location[i] - Player->GetTransform().Location[i]);
		}
		if (Math::VectorSize(Raw) < Player->GetLength() * 1.5f)
		{
			bool collisionX = Player->GetTransform().Location[0] + Player->GetLength() >= (*it)->GetTransform().Location[0] &&
				(*it)->GetTransform().Location[0] + (*it)->GetLength() >= Player->GetTransform().Location[0];
			// collision y-axis?
			bool collisionY = Player->GetTransform().Location[1] + Player->GetLength() >= (*it)->GetTransform().Location[1] &&
				(*it)->GetTransform().Location[1] + (*it)->GetLength() >= Player->GetTransform().Location[1];
			// collision only if on both axes
			if (collisionX && collisionY)
			{
				if ((*it)->GetIsHazard())
				{
					std::cout << "Collision with hazard!" << std::endl;
					Player->MarkedForDelete = true;
					Player = nullptr;
				}
				else
				{
					std::cout << "Collision with item" << std::endl;
					(*it)->MarkedForDelete = true;
				}
			}
		}
	}
}

void World::HandleCollision(GameObject& GameObject1, GameObject& GameObject2)
{
	if (GameObject2.GetIsHazard())
	{
		Player = nullptr;
	}
	else
	{
		delete& GameObject2;
	}
}





void World::Update(float DeltaSeconds)
{
	GarbageCollection();
	InputUpdate();
	RenderUpdate();
	glfwPollEvents();
	//CollisionUpdate();

}

bool World::IsRunning()
{
	return !glfwWindowShouldClose(Window);
}


