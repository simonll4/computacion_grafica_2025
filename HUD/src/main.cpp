extern "C"{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gfx/SkyboxRenderer.h"
#include "gfx/TextureCube.h"
#include "gfx/SimpleCube.h"
#include "gfx/TerrainRenderer.h"
#include "hud/FlightHUD.h"
#include "flight/FlightData.h"

static const char* kWindowTitle = "Flight Simulator HUD - OpenGL";
static const int kWindowWidth   = 1280;
static const int kWindowHeight  = 720;

// Variables globales para la cámara
// Iniciar en el piso (altitud 0 ft = altura de ojos)
glm::vec3 cameraPos = glm::vec3(0.0f, 1.8f, 0.0f);  // Y = 1.8m (altura de ojos, altitud 0 ft)
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = kWindowWidth / 2.0f;
float lastY = kWindowHeight / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Variables del simulador de vuelo
flight::FlightData flightData;
hud::FlightHUD* globalHUD = nullptr;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
void print_gl_version(void);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	GLFWwindow* window = glfwCreateWindow(kWindowWidth,
					      kWindowHeight,
					      kWindowTitle, nullptr, nullptr);
	
	if (window == nullptr){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	// https://stackoverflow.com/questions/48650497/glad-failing-to-initialize
	glfwMakeContextCurrent(window);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	print_gl_version();

	glViewport(0, 0, kWindowWidth, kWindowHeight);
	
	// Configurar callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	// Habilitar depth testing
	glEnable(GL_DEPTH_TEST);
	
	// Crear skybox
	gfx::TextureCube cubemap;
	gfx::SkyboxRenderer skybox;
	
	// Crear cubo de referencia
	gfx::SimpleCube cube;
	
	// Crear terreno
	gfx::TerrainRenderer terrain;
	gfx::TerrainParams terrainParams;
	
	// Crear HUD
	hud::FlightHUD flightHUD;
	globalHUD = &flightHUD;
	
	try {
		// Cargar el primer atlas del cubemap
		if (!cubemap.loadFromAtlas("Cubemap/Cubemap_Sky_01-512x512.png", false)) {
			std::cerr << "Failed to load cubemap atlas" << std::endl;
			return -1;
		}
		
		skybox.init();
		skybox.setCubemap(&cubemap);
		
		// Inicializar cubo
		cube.init();
		
		// Inicializar terreno
		terrain.init();
		terrain.loadTextures("forrest_ground_01_4k.blend/textures");
		terrainParams.groundY = 0.0f;
		terrainParams.tileScaleMacro = 0.05f;   // Ajustar para que se vea bien
		terrainParams.tileScaleDetail = 0.4f;
		terrainParams.detailStrength = 0.3f;
		terrainParams.fogDensity = 0.00f; // No hay niebla
		
		// Inicializar HUD
		flightHUD.init(kWindowWidth, kWindowHeight);
		flightHUD.setLayout("classic");
		
		std::cout << "Skybox, terrain and HUD initialized successfully!" << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Error initializing systems: " << e.what() << std::endl;
		return -1;
	}
	
	while(!glfwWindowShouldClose(window)){
		// Calcular deltaTime
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		// Input
		processInput(window);
		
		// Actualizar datos de vuelo basados en la cámara
		flightData.updateFromCamera(cameraFront, cameraUp, cameraPos, deltaTime);
		flightData.simulatePhysics(deltaTime);
		
		// Obtener tamaño actual del framebuffer
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		
		// Actualizar HUD si cambió el tamaño de ventana
		static int lastWidth = width, lastHeight = height;
		if (width != lastWidth || height != lastHeight) {
			flightHUD.setScreenSize(width, height);
			lastWidth = width;
			lastHeight = height;
		}
		
		// Limpiar buffers
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Crear matrices de vista y proyección
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
												(float)width / (float)height, 
												0.1f, 1000.0f);  // Far plane más lejano para ver terreno
		
		// Renderizar skybox (3D)
		skybox.draw(view, projection);
		
		// Renderizar terreno
		terrain.draw(view, projection, cameraPos, terrainParams);
		
		// Renderizar cubo de referencia en varias posiciones
		cube.draw(view, projection, glm::vec3(0.0f, 0.0f, -5.0f));  // Adelante
		cube.draw(view, projection, glm::vec3(5.0f, 0.0f, -5.0f));  // Derecha
		cube.draw(view, projection, glm::vec3(-5.0f, 0.0f, -5.0f)); // Izquierda
		cube.draw(view, projection, glm::vec3(0.0f, 3.0f, -5.0f));  // Arriba
		
		// Actualizar y renderizar HUD (2D overlay)
		flightHUD.update(flightData);
		flightHUD.render();
		// Swap buffers y poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	// Limpiar recursos explícitamente antes de terminar GLFW
	std::cout << "Cleaning up resources..." << std::endl;
	
	// No hacer nada más, los destructores se encargan
	// No es necesario llamar a glfwTerminate() explícitamente
	// ya que GLFW se encarga de liberar los recursos cuando se cierra
	// la ventana principal
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	// Controles de cámara (simulación de vuelo)
	float cameraSpeed = 10.0f * deltaTime;  // Aumentada de 2.5 a 10.0
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	
	// Controles de altitud
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		cameraPos += cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f); // Subir
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		cameraPos -= cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f); // Bajar
	
	// Colisión con el piso (altura de ojos del piloto sobre el suelo)
	const float GROUND_LEVEL = 1.8f;  // 1.8 metros (altura de ojos)
	if (cameraPos.y < GROUND_LEVEL) {
		cameraPos.y = GROUND_LEVEL;
	}
	
	// Controles de HUD (con debounce simple)
	static float lastLayoutChange = 0.0f;
	float currentTime = glfwGetTime();
	
	if (currentTime - lastLayoutChange > 0.5f) { // Debounce de 0.5 segundos
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			if (globalHUD) globalHUD->setLayout("classic");
			lastLayoutChange = currentTime;
			std::cout << "HUD Layout: Classic" << std::endl;
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			if (globalHUD) globalHUD->setLayout("modern");
			lastLayoutChange = currentTime;
			std::cout << "HUD Layout: Modern" << std::endl;
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
			if (globalHUD) globalHUD->setLayout("minimal");
			lastLayoutChange = currentTime;
			std::cout << "HUD Layout: Minimal" << std::endl;
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // Invertido porque las coordenadas y van de abajo hacia arriba
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Limitar el pitch
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	// Calcular nueva dirección de la cámara
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void print_gl_version(void){
    // get version info
     const GLubyte* renderer = glGetString(GL_RENDERER);    // get renderer string
     const GLubyte* version = glGetString(GL_VERSION);      // version as a string

     std::cout << "Renderer: " << renderer << std::endl;
     std::cout << "OpenGL version supported " << version << std::endl;
}
