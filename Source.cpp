#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <cstdlib> 
#include <random>
#include <iostream>
#include <windows.h> 
#include "stb_image.h"
#include "Shader.h"
#include <filesystem> 
#include "GameObject.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::vector<GameObject*> AllObjects;
GameObject* Player;
GLFWwindow* InitWindow();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void DrawObjects(std::vector<Shader>& InShaderList);
void UpdateObjects();
static int RandomInRange(int InMaxNumber);

using u32 = uint_least32_t;
using engine = std::mt19937;
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
	// window
	GLFWwindow* window = InitWindow();


	// build and compile our texture shader
	// ------------------------------------
	Shader ourShader("Texture.vert", "Texture.frag");
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// positions          // colors           // texture coords
		 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
			0, 1, 3, // first triangle
			1, 2, 3  // second triangle
	};
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// load and create a texture 
	// -------------------------
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	stbi_set_flip_vertically_on_load(1);
	unsigned char* data = stbi_load("grass2.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	/////////////
	Player = new GameObject();
	AllObjects.push_back(Player);
	// player triangle
	 // build and compile our shader program
	// ------------------------------------


	std::vector<Shader> Shaders;
	DrawObjects(Shaders);
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// bind Texture
		glBindTexture(GL_TEXTURE_2D, texture);
		// render container
		ourShader.use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//std::cout << Player->ActiveInput[0];
		//std::cout << ",";
		//std::cout <<  Player->ActiveInput[1] << std::endl;

		UpdateObjects();
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	delete Player;
	// should use shared ptr - this ( the world ) will keep objects alive as long as it's alive
	AllObjects.clear();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	std::vector<float> Input = { 0.f,0.f };
	/// player input for movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		Input[1] = 1.f;// Player->ConsumeInput({ 0.0f,1.0f });
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Input[0] = -1.f;// Player->ConsumeInput({ -1.0f,0.0f });
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Input[0] = 1.f;	//Player->ConsumeInput({ 1.0f,0.0f });
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		Input[1] = -1.f;//Player->ConsumeInput({ 0.0f,-1.0f });

	Player->ConsumeInput(Input);



}

void DrawObjects(std::vector<Shader>& InShaderList)
{
	std::vector<Shader> Shaders;
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	for (auto it = AllObjects.begin(); it != AllObjects.end(); ++it)
	{
		//Shader TriShader("Shader.vert", "Shader.frag"); // you can name your shader files however you like
		InShaderList.emplace_back("Shader.vert", "Shader.frag"); // Use emplace_back to construct in-place
		Shader& TriShader = InShaderList.back(); // Get reference to the newly added shader
		float vertices[] = {
			// positions         // colors
			(*it)->WorldTransform.Location[0] + (*it)->Length * std::cos((*it)->WorldTransform.Rotation - (3 * PI / 4.0f)), (*it)->WorldTransform.Location[1] + (*it)->Length * std::sin((*it)->WorldTransform.Rotation - (3 * PI / 4.0f)), 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
			(*it)->WorldTransform.Location[0] + (*it)->Length * std::cos((*it)->WorldTransform.Rotation + (3*PI/4.0f))    , (*it)->WorldTransform.Location[1] + (*it)->Length * std::sin((*it)->WorldTransform.Rotation + (3 * PI / 4.0f)), 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
			(*it)->WorldTransform.Location[0] + (*it)->Length * std::cos((*it)->WorldTransform.Rotation)                  , (*it)->WorldTransform.Location[1] + (*it)->Length * std::sin((*it)->WorldTransform.Rotation)                  , 0.0f,  0.0f, 0.0f, 1.0f   // top 
		};


	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(VAO);
	TriShader.VAO = VAO;
	TriShader.use();
	(*it)->ObjectShader = &TriShader;
	glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}

void UpdateObjects()
{
	for (auto it = AllObjects.begin(); it != AllObjects.end(); ++it)
	{
		// create transformations
		glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		transform = glm::translate(transform, glm::vec3((*it)->WorldTransform.Location[0], (*it)->WorldTransform.Location[1], 0.0f));
		transform = glm::rotate(transform, (*it)->WorldTransform.Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		(*it)->ActiveInput;
		// get matrix's uniform location and set matrix
		(*it)->ObjectShader->use();
		unsigned int transformLoc = glGetUniformLocation((*it)->ObjectShader->ID, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));


		glBindVertexArray((*it)->ObjectShader->VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}

int RandomInRange(int InMaxNumber)
{
	std::random_device os_seed;
	const u32 seed = os_seed();
	engine generator(seed);
	std::uniform_int_distribution< u32 > distribute(0, InMaxNumber);
	return distribute(generator);
}

GLFWwindow* InitWindow()
{
	// glfw: initialize and configure
// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Ast", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}
	return window;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}