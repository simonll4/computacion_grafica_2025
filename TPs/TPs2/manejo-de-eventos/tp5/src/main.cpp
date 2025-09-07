// src/main.cpp — TP5 Guía 2 (Manejo de eventos)
// Triángulo que puede seguir el cursor del mouse en tiempo real.
// Tecla 'M': activa/desactiva el modo de seguimiento del mouse.
// Tecla 'R': resetea a la posición original.

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

// ---------------- window ----------------
static const char *kTitle = "TP5 - Triangulo que sigue el mouse (GL core)";
static int kW = 600, kH = 600;

// ---------------- globals ----------------
static GLFWwindow *gWindow = nullptr;
static Shader *gProg = nullptr;
static SimpleMesh *gMesh = nullptr;

static glm::mat4 gProj(1.0f);

// Triángulo MÁS CHICO (NDC)
static const glm::vec2 kTopVertex(0.0f, 0.5f); // vértice superior en modelo
static const glm::vec2 kLeftBase(-0.35f, -0.2f);
static const glm::vec2 kRightBase(0.35f, -0.2f);

// Pivote actual (NDC) — por consigna, arranca en el VÉRTICE SUPERIOR
static glm::vec2 gPivot = kTopVertex;

// Pre-traslación que coloca el top-vertex en gPivot (anclaje)
static glm::vec2 gPreTranslation = (gPivot - kTopVertex);

// Variables para seguimiento del mouse
static bool gFollowMouse = false;
static glm::vec2 gMousePosition = kTopVertex;

static float gAngularSpeed = glm::radians(90.0f); // rad/s
static bool gPaused = false;
static double gLastTime = 0.0;
static float gAngle = 0.0f;

// ---------------- util ----------------
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
static glm::vec2 screenToNDC(double mx, double my)
{
	float x_ndc = 2.0f * float(mx) / float(kW) - 1.0f;
	float y_ndc = 1.0f - 2.0f * float(my) / float(kH);
	return glm::vec2(x_ndc, y_ndc);
}

// ---------------- callbacks ----------------
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
			// Toggle mouse following mode
			gFollowMouse = !gFollowMouse;
			if (gFollowMouse)
			{
				std::cout << "Modo seguimiento del mouse ACTIVADO\n";
				// Obtener posición actual del mouse
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
			// Reset: top vertex como pivote y triángulo vuelve a anclarse ahí.
			gPivot = kTopVertex;
			gPreTranslation = (gPivot - kTopVertex); // = (0,0)
			gAngle = 0.0f;
			gFollowMouse = false; // También desactivar el seguimiento del mouse
			std::cout << "Reset: triángulo en posición original\n";
		}
	}
}
static void mouse_button_callback(GLFWwindow *win, int button, int action, int)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double mx, my;
		glfwGetCursorPos(win, &mx, &my);
		gPivot = screenToNDC(mx, my);
		// Reposicionar el triángulo para que su vértice superior coincida con el nuevo pivote.
		gPreTranslation = (gPivot - kTopVertex);
		std::cout << "Nuevo pivote (NDC): (" << gPivot.x << ", " << gPivot.y << ")\n";
	}
}

static void cursor_position_callback(GLFWwindow *win, double xpos, double ypos)
{
	if (gFollowMouse)
	{
		gMousePosition = screenToNDC(xpos, ypos);
		// Posicionar el triángulo debajo del cursor
		gPivot = gMousePosition;
		gPreTranslation = (gPivot - kTopVertex);
	}
}

// ---------------- init triangle mesh ----------------
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

// ---------------- main ----------------
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

		// delta time + ángulo acumulado
		double now = glfwGetTime();
		double dt = now - gLastTime;
		gLastTime = now;
		if (!gPaused)
		{
			gAngle += gAngularSpeed * static_cast<float>(dt);
			if (gAngle > 1000.0f)
				gAngle = std::fmod(gAngle, 2.0f * float(M_PI));
		}

		//   M = T(pivot) * R(angle) * T(-pivot) * T(pre)
		// donde T(pre) traslada el triángulo para que su top-vertex = pivot ANTES de rotar.
		// Cuando gFollowMouse es true, el triángulo se posiciona debajo del cursor.
		glm::mat4 M =
			translate2D(gPivot) *
			rotateZ(gAngle) *
			translate2D(-gPivot) *
			translate2D(gPreTranslation);

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
