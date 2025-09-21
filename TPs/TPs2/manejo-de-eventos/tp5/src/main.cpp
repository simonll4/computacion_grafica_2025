/*
 * TP5 - Seguimiento del Mouse en Tiempo Real
 * 
 * Este programa implementa un triángulo que puede seguir el cursor del mouse
 * 
 * FUNCIONALIDADES:
 * 1. Rotación continua alrededor de un pivote 
 * 2. Click izquierdo para establecer nuevo pivote
 * 3. NUEVO: Seguimiento del mouse en tiempo real
 * 
 * MODO SEGUIMIENTO:
 * - Cuando está activado, el triángulo sigue al cursor automáticamente
 * - El vértice superior se posiciona donde está el cursor
 * - La rotación continúa mientras sigue al mouse
 * 
 * CONTROLES:
 *   M: Activar/desactivar seguimiento del mouse
 *   Click izquierdo: Establecer nuevo pivote (modo manual)
 *   R: Reset a posición original
 *   ESPACIO: Pausar/reanudar rotación
 *   ESC: Salir
 * 
 * COMPILACIÓN:
 *   make
 *   ./build/OGL-program
 */

extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cmath>

#include "Shader.h"
#include "SimpleMesh.h"

// Configuración de ventana
static const char *kTitle = "TP5 - Triangulo que sigue el mouse (GL core)";
static int kW = 600, kH = 600;

// Variables globales
static GLFWwindow *gWindow = nullptr;
static Shader *gProg = nullptr;
static SimpleMesh *gMesh = nullptr;

static glm::mat4 gProj(1.0f);

// Geometría del triángulo en coordenadas NDC
static const glm::vec2 kTopVertex(0.0f, 0.5f);    // Vértice superior
static const glm::vec2 kLeftBase(-0.35f, -0.2f);  // Base izquierda
static const glm::vec2 kRightBase(0.35f, -0.2f);  // Base derecha

// Estado de la animación y posicionamiento
static glm::vec2 gPivot = kTopVertex;              // Pivote actual (inicia en vértice superior)
static glm::vec2 gPreTranslation = (gPivot - kTopVertex); // Traslación para anclar al pivote

// Variables para seguimiento del mouse
static bool gFollowMouse = false;                  // Estado del modo seguimiento
static glm::vec2 gMousePosition = kTopVertex;      // Última posición del mouse en NDC

static float gAngularSpeed = glm::radians(90.0f); // Velocidad angular (90°/s)
static bool gPaused = false;                       // Estado de pausa
static double gLastTime = 0.0;                     // Último tiempo registrado
static float gAngle = 0.0f;                        // Ángulo acumulado

// Funciones auxiliares
static inline glm::mat4 translate2D(const glm::vec2 &t)
{
	return glm::translate(glm::mat4(1.0f), glm::vec3(t, 0.0f));
}
static inline glm::mat4 rotateZ(float radians)
{
	return glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0, 0, 1));
}
static void updateProjection()
{
	gProj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
}
// Convierte coordenadas de pantalla a NDC (Normalized Device Coordinates)
static glm::vec2 screenToNDC(double mx, double my)
{
	float x_ndc = 2.0f * float(mx) / float(kW) - 1.0f;  // [0,kW] -> [-1,1]
	float y_ndc = 1.0f - 2.0f * float(my) / float(kH);  // [0,kH] -> [1,-1] (Y invertida)
	return glm::vec2(x_ndc, y_ndc);
}

// Callbacks de eventos
static void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
	kW = (w > 1) ? w : 1;
	kH = (h > 1) ? h : 1;
	glViewport(0, 0, kW, kH);
	updateProjection();
}
static void key_callback(GLFWwindow *win, int key, int, int action, int)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(win, GLFW_TRUE);
		if (key == GLFW_KEY_SPACE)
			gPaused = !gPaused;
		if (key == GLFW_KEY_M)
		{
			// Alternar modo de seguimiento del mouse
			gFollowMouse = !gFollowMouse;
			if (gFollowMouse)
			{
				std::cout << "Modo seguimiento del mouse ACTIVADO\n";
				// Inicializar con la posición actual del cursor
				double mx, my;
				glfwGetCursorPos(win, &mx, &my);
				gMousePosition = screenToNDC(mx, my);
				gPivot = gMousePosition;
				gPreTranslation = (gPivot - kTopVertex);
			}
			else
			{
				std::cout << "Modo seguimiento del mouse DESACTIVADO\n";
			}
		}
		if (key == GLFW_KEY_R)
		{
			// Reset completo: volver al estado inicial
			gPivot = kTopVertex;
			gPreTranslation = (gPivot - kTopVertex); // = (0,0)
			gAngle = 0.0f;
			gFollowMouse = false; // Desactivar seguimiento del mouse
			std::cout << "Reset: triángulo en posición original\n";
		}
	}
}
// Callback para clicks del mouse (modo manual)
static void mouse_button_callback(GLFWwindow *win, int button, int action, int)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		// Solo funciona si no está en modo seguimiento
		if (!gFollowMouse)
		{
			double mx, my;
			glfwGetCursorPos(win, &mx, &my);
			gPivot = screenToNDC(mx, my);
			gPreTranslation = (gPivot - kTopVertex);
			std::cout << "Nuevo pivote (NDC): (" << gPivot.x << ", " << gPivot.y << ")\n";
		}
	}
}

// Callback para movimiento del cursor (modo seguimiento)
static void cursor_position_callback(GLFWwindow *win, double xpos, double ypos)
{
	if (gFollowMouse)
	{
		// Actualizar posición del mouse en NDC
		gMousePosition = screenToNDC(xpos, ypos);
		// Posicionar el triángulo para que su vértice superior siga al cursor
		gPivot = gMousePosition;
		gPreTranslation = (gPivot - kTopVertex);
	}
}

// Inicialización de la geometría
static SimpleMesh *makeTriangleMesh()
{
	// Posiciones solo (tu SimpleMesh recibe positions + indices)
	std::vector<float> positions = {
		kTopVertex.x, kTopVertex.y,
		kLeftBase.x, kLeftBase.y,
		kRightBase.x, kRightBase.y};
	std::vector<unsigned> indices = {0, 1, 2};
	return new SimpleMesh(positions, true, indices);
}

// Función principal
int main()
{
	if (!glfwInit())
	{
		std::cerr << "Error: glfwInit()\n";
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	gWindow = glfwCreateWindow(kW, kH, kTitle, nullptr, nullptr);
	if (!gWindow)
	{
		std::cerr << "Error: createWindow\n";
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(gWindow);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Error: gladLoadGLLoader\n";
		return 1;
	}

	glfwSetFramebufferSizeCallback(gWindow, framebuffer_size_callback);
	glfwSetKeyCallback(gWindow, key_callback);
	glfwSetMouseButtonCallback(gWindow, mouse_button_callback);
	glfwSetCursorPosCallback(gWindow, cursor_position_callback);

	glViewport(0, 0, kW, kH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader prog("shaders/basic.vert", "shaders/basic.frag");
	gProg = &prog;

	gMesh = makeTriangleMesh();
	updateProjection();

	// Mostrar instrucciones
	std::cout << "=== TP5 - Triangulo que sigue el mouse ===\n";
	std::cout << "Controles:\n";
	std::cout << "  M - Activar/desactivar seguimiento del mouse\n";
	std::cout << "  R - Reset a posición original\n";
	std::cout << "  SPACE - Pausar/reanudar rotación\n";
	std::cout << "  ESC - Salir\n";
	std::cout << "==========================================\n";

	gLastTime = glfwGetTime();

	while (!glfwWindowShouldClose(gWindow))
	{
		glfwPollEvents();

		// Actualizar ángulo de rotación
		double now = glfwGetTime();
		double dt = now - gLastTime;
		gLastTime = now;
		if (!gPaused)
		{
			gAngle += gAngularSpeed * static_cast<float>(dt);
			// Normalizar ángulo para evitar overflow
			if (gAngle > 1000.0f)
				gAngle = std::fmod(gAngle, 2.0f * float(M_PI));
		}

		// Construir matriz de transformación compuesta:
		// M = T(pivot) * R(angle) * T(-pivot) * T(pre)
		// En modo seguimiento: el pivote se actualiza continuamente con la posición del cursor
		// En modo manual: el pivote se establece con clicks del mouse
		glm::mat4 M =
			translate2D(gPivot) *        // 4. Mover a posición final (cursor o click)
			rotateZ(gAngle) *            // 3. Rotar
			translate2D(-gPivot) *       // 2. Centrar en origen
			translate2D(gPreTranslation); // 1. Anclar vértice al pivote

		glm::mat4 MVP = gProj * M;

		glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		gProg->use();
		gProg->setMat4("uMVP", glm::value_ptr(MVP));
		gMesh->bind();
		gMesh->draw();

		glfwSwapBuffers(gWindow);
	}

	delete gMesh;
	glfwDestroyWindow(gWindow);
	glfwTerminate();
	return 0;
}
