/**
 * @file main.cpp
 * @brief Simulador de vuelo con HUD (Heads-Up Display) profesional
 *
 * Sistema completo de renderizado 3D con:
 * - Terreno con texturizado triplanar y niebla
 * - Skybox para cielo envolvente
 * - HUD con altímetro de 7 segmentos
 * - Sistema de cámara libre tipo FPS
 * - Física básica de vuelo
 */

extern "C"
{
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

// ============================================================================
// CONSTANTES DE CONFIGURACIÓN
// ============================================================================

static const char *kWindowTitle = "Flight Simulator HUD - OpenGL";
static const int kWindowWidth = 1280;
static const int kWindowHeight = 720;

// Altura mínima de la cámara (1.8m = altura de ojos del piloto)
static const float kGroundLevel = 1.8f;

// Velocidad de movimiento de la cámara (m/s)
static const float kCameraSpeed = 10.0f;

// Sensibilidad del mouse para rotación de cámara
static const float kMouseSensitivity = 0.1f;

// ============================================================================
// ESTADO GLOBAL DE LA CÁMARA
// ============================================================================

// Posición inicial: en el piso (Y=1.8m = altitud 0 pies)
glm::vec3 cameraPos = glm::vec3(0.0f, kGroundLevel, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Mirando hacia -Z
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);	  // Eje Y arriba

// Ángulos de Euler para rotación de cámara
float yaw = -90.0f; // Inicializado hacia -Z
float pitch = 0.0f; // Mirando al horizonte

// Mouse state para cálculo de delta
float lastX = kWindowWidth / 2.0f;
float lastY = kWindowHeight / 2.0f;
bool firstMouse = true;

// ============================================================================
// TIMING
// ============================================================================

float deltaTime = 0.0f; // Tiempo entre frames
float lastFrame = 0.0f; // Timestamp del frame anterior

// ============================================================================
// DATOS DE SIMULACIÓN
// ============================================================================

flight::FlightData flightData;		 // Datos del avión (velocidad, altitud, etc.)
hud::FlightHUD *globalHUD = nullptr; // Puntero global al HUD (para callbacks)

// ============================================================================
// DECLARACIÓN DE FUNCIONES
// ============================================================================

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);
void print_gl_version(void);

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

/**
 * @brief Punto de entrada del programa
 *
 * Inicializa todos los sistemas (ventana, OpenGL, recursos gráficos)
 * y ejecuta el loop principal de renderizado.
 */
int main()
{
	// ------------------------------------------------------------------------
	// 1. INICIALIZACIÓN DE GLFW Y VENTANA
	// ------------------------------------------------------------------------

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(
		kWindowWidth,
		kWindowHeight,
		kWindowTitle,
		nullptr,
		nullptr);

	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// ------------------------------------------------------------------------
	// 2. INICIALIZACIÓN DE GLAD (OpenGL Function Loader)
	// ------------------------------------------------------------------------

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	print_gl_version();

	// ------------------------------------------------------------------------
	// 3. CONFIGURACIÓN DE OPENGL
	// ------------------------------------------------------------------------

	glViewport(0, 0, kWindowWidth, kWindowHeight);
	glEnable(GL_DEPTH_TEST); // Habilitar test de profundidad para 3D

	// ------------------------------------------------------------------------
	// 4. CONFIGURACIÓN DE CALLBACKS
	// ------------------------------------------------------------------------

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Mouse capturado

	// ------------------------------------------------------------------------
	// 5. CREACIÓN DE OBJETOS DE RENDERIZADO
	// ------------------------------------------------------------------------

	gfx::TextureCube cubemap;		  // Textura del skybox (6 caras)
	gfx::SkyboxRenderer skybox;		  // Renderizador del cielo
	gfx::SimpleCube cube;			  // Cubos de referencia
	gfx::TerrainRenderer terrain;	  // Renderizador del terreno
	gfx::TerrainParams terrainParams; // Parámetros del terreno
	hud::FlightHUD flightHUD;		  // Sistema de HUD

	globalHUD = &flightHUD; // Guardar puntero global para callbacks

	// ------------------------------------------------------------------------
	// 6. INICIALIZACIÓN DE RECURSOS GRÁFICOS
	// ------------------------------------------------------------------------

	try
	{
		// Skybox: cargar atlas y compilar shaders
		if (!cubemap.loadFromAtlas("Cubemap/Cubemap_Sky_01-512x512.png", false))
		{
			std::cerr << "Failed to load cubemap atlas" << std::endl;
			return -1;
		}
		skybox.init();
		skybox.setCubemap(&cubemap);

		// Cubos de referencia: crear geometría
		cube.init();

		// Terreno: generar mesh, cargar texturas
		terrain.init();
		terrain.loadTextures("forrest_ground_01_4k.blend/textures");

		// Configurar parámetros del terreno
		terrainParams.groundY = 0.0f;		  // Nivel del piso
		terrainParams.tileScaleMacro = 0.05f; // Escala textura principal
		terrainParams.tileScaleDetail = 0.4f; // Escala textura de detalle
		terrainParams.detailStrength = 0.3f;  // Mezcla de detalle (0-1)
		terrainParams.fogDensity = 0.00f;	  // Niebla deshabilitada

		// HUD: compilar shaders, inicializar altímetro
		flightHUD.init(kWindowWidth, kWindowHeight);
		flightHUD.setLayout("classic");

		std::cout << "✓ All systems initialized successfully!" << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "✗ Error initializing systems: " << e.what() << std::endl;
		return -1;
	}

	// ------------------------------------------------------------------------
	// 7. LOOP PRINCIPAL DE RENDERIZADO
	// ------------------------------------------------------------------------

	while (!glfwWindowShouldClose(window))
	{

		// --- Timing ---
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// --- Input ---
		processInput(window);

		// --- Actualización de lógica ---
		flightData.updateFromCamera(cameraFront, cameraUp, cameraPos, deltaTime);
		flightData.simulatePhysics(deltaTime);

		// --- Manejo de resize de ventana ---
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		static int lastWidth = width, lastHeight = height;
		if (width != lastWidth || height != lastHeight)
		{
			flightHUD.setScreenSize(width, height);
			lastWidth = width;
			lastHeight = height;
		}

		// --- Limpiar buffers ---
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --- Matrices de cámara ---
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f),
			(float)width / (float)height,
			0.1f,	// Near plane
			1000.0f // Far plane (lejano para ver terreno)
		);

		// --- Renderizado 3D ---
		skybox.draw(view, projection);
		terrain.draw(view, projection, cameraPos, terrainParams);

		// Cubo de referencia
		cube.draw(view, projection, glm::vec3(0.0f, 0.0f, 5.0f)); // Adelante

		// --- Renderizado 2D (HUD overlay) ---
		flightHUD.update(flightData);
		flightHUD.render();

		// --- Swap y eventos ---
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ------------------------------------------------------------------------
	// 8. LIMPIEZA Y CIERRE
	// ------------------------------------------------------------------------

	std::cout << "Cleaning up resources..." << std::endl;
	// Los destructores de C++ se encargan de liberar los recursos automáticamente

	return 0;
}

// ============================================================================
// CALLBACKS
// ============================================================================

/**
 * @brief Callback para ajustar viewport cuando se redimensiona la ventana
 */
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

/**
 * @brief Procesa entrada de teclado para controlar la cámara y HUD
 *
 * Controles:
 * - ESC: Cerrar aplicación
 * - W/A/S/D: Movimiento horizontal
 * - Q/E: Subir/bajar (con límite en el piso)
 * - 1/2/3: Cambiar layout del HUD
 */
void processInput(GLFWwindow *window)
{
	// --- Salida ---
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	// --- Movimiento de cámara (tipo vuelo libre) ---
	float speed = kCameraSpeed * deltaTime;
	glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // Adelante
		cameraPos += speed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // Atrás
		cameraPos -= speed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // Izquierda
		cameraPos -= speed * right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // Derecha
		cameraPos += speed * right;

	// --- Controles de altitud ---
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // Subir
		cameraPos.y += speed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // Bajar
		cameraPos.y -= speed;

	// --- Colisión con el piso ---
	if (cameraPos.y < kGroundLevel)
	{
		cameraPos.y = kGroundLevel;
	}

	// --- Controles de HUD (con anti-rebote) ---
	static float lastLayoutChange = 0.0f;
	float currentTime = glfwGetTime();

	if (currentTime - lastLayoutChange > 0.5f)
	{
		// TODO sin manejo de layout por ahora
		//  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		//  {
		//  	if (globalHUD)
		//  		globalHUD->setLayout("classic");
		//  	lastLayoutChange = currentTime;
		//  	std::cout << "HUD Layout: Classic" << std::endl;
		//  }
		//  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		//  {
		//  	if (globalHUD)
		//  		globalHUD->setLayout("modern");
		//  	lastLayoutChange = currentTime;
		//  	std::cout << "HUD Layout: Modern" << std::endl;
		//  }
		//  if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		//  {
		//  	if (globalHUD)
		//  		globalHUD->setLayout("minimal");
		//  	lastLayoutChange = currentTime;
		//  	std::cout << "HUD Layout: Minimal" << std::endl;
		//  }
	}
}

/**
 * @brief Callback para manejar movimiento del mouse y rotación de cámara
 *
 * Implementa rotación tipo FPS usando ángulos de Euler (yaw/pitch).
 * El pitch está limitado a ±89° para evitar gimbal lock.
 */
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	// Inicialización en primer frame
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	// Calcular offset del mouse
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // Invertido: Y crece hacia abajo en pantalla
	lastX = xpos;
	lastY = ypos;

	// Aplicar sensibilidad
	xoffset *= kMouseSensitivity;
	yoffset *= kMouseSensitivity;

	// Actualizar ángulos de Euler
	yaw += xoffset;
	pitch += yoffset;

	// Limitar pitch para evitar flip vertical (gimbal lock)
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	// Calcular vector dirección usando ángulos de Euler
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

/**
 * @brief Imprime información de OpenGL (renderer, versión)
 */
void print_gl_version(void)
{
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);

	std::cout << "Renderer: " << renderer << std::endl;
	std::cout << "OpenGL version supported: " << version << std::endl;
}
