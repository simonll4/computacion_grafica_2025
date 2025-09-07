// main.cpp — TP2 Guía 2 (versión simple: sólo SPACE para pausar)
// Animaciones: rotación alrededor del vértice superior (1),
// pulso alrededor del centroide (2),
// traslación rígida con el vértice superior moviéndose en circunferencia (3).

extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

#include "Shader.h"
#include "SimpleMesh.h"
#include "Ortho.h"

// ---------------- Config ventana ----------------
static const char *kTitle = "TP2 - Guía 2 (GL core) - Simple";
static int kW = 1200, kH = 1200;

// Modo de animación (elegido por teclado 1/2/3)
enum class Mode
{
	RotatePivot = 1,
	Pulse = 2,
	RigidCircle = 3
};

// Estado global (sólo pausa/tiempos)
struct AppState
{
	Mode mode = Mode::RotatePivot;
	bool paused = false;
	double t0 = 0.0;	 // tiempo base (inicio o al salir de pausa)
	double tPause = 0.0; // instante en el que se pausó
} gState;

// ---------------- Triángulo en espacio local ----------------
static const std::array<float, 9> kTri = {
	0.0f, 0.3f, 0.0f,	 // v0: TOP (pivote)
	-0.25f, -0.2f, 0.0f, // v1
	0.25f, -0.2f, 0.0f	 // v2
};

static glm::vec2 topLocal()
{
	return {kTri[0], kTri[1]};
}
static glm::vec2 centroid()
{
	return {
		(kTri[0] + kTri[3] + kTri[6]) / 3.0f,
		(kTri[1] + kTri[4] + kTri[7]) / 3.0f};
}

// ---------------- Callbacks / helpers ----------------
static void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
	kW = w;
	kH = h;
	glViewport(0, 0, w, h);
}

static void print_gl()
{
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "OpenGL:   " << glGetString(GL_VERSION) << "\n";
}

// Input: ESC, 1/2/3, SPACE pausa (sin R/Q/E)
static void processInput(GLFWwindow *win)
{
	if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(win, GLFW_TRUE);

	if (glfwGetKey(win, GLFW_KEY_1) == GLFW_PRESS)
		gState.mode = Mode::RotatePivot;
	if (glfwGetKey(win, GLFW_KEY_2) == GLFW_PRESS)
		gState.mode = Mode::Pulse;
	if (glfwGetKey(win, GLFW_KEY_3) == GLFW_PRESS)
		gState.mode = Mode::RigidCircle;

	static bool spacePressed = false;
	int st = glfwGetKey(win, GLFW_KEY_SPACE);
	if (st == GLFW_PRESS && !spacePressed)
	{
		spacePressed = true;
		if (!gState.paused)
		{
			gState.paused = true;
			gState.tPause = glfwGetTime();
		}
		else
		{
			gState.paused = false;
			gState.t0 += (glfwGetTime() - gState.tPause); // corrige salto al reanudar
		}
	}
	if (st == GLFW_RELEASE)
		spacePressed = false;
}

// Tiempo “virtual” sin escalado (respeta la pausa)
static float tNow()
{
	const double now = glfwGetTime();
	const double t = gState.paused ? (gState.tPause - gState.t0)
								   : (now - gState.t0);
	return float(t);
}

// ---------------- Programa principal ----------------
int main()
{
	if (!glfwInit())
	{
		std::cerr << "GLFW init failed\n";
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *win = glfwCreateWindow(kW, kH, kTitle, nullptr, nullptr);
	if (!win)
	{
		std::cerr << "Window creation failed\n";
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(win);
	glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "GLAD load failed\n";
		return 1;
	}
	print_gl();

	std::vector<float> tri(kTri.begin(), kTri.end());
	SimpleMesh mesh(tri, /*useIndices*/ false);

	Shader shader("shaders/basic.vert", "shaders/basic.frag");

	glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
	gState.t0 = glfwGetTime();

	const glm::vec2 pTop = topLocal();
	const glm::vec2 c = centroid();
	const glm::mat4 V(1.0f);

	while (!glfwWindowShouldClose(win))
	{
		processInput(win);
		glClear(GL_COLOR_BUFFER_BIT);

		const float t = tNow();

		// Proyección ortográfica dependiente del aspect
		glm::mat4 P = orthoAspect((float)kW, (float)kH, 0.8f);

		glm::mat4 M(1.0f);
		if (gState.mode == Mode::RotatePivot)
		{
			// M = T(pTop) * Rz(wt) * T(-pTop)
			const float w = 1.2f; // rad/s
			M = glm::translate(M, glm::vec3(pTop, 0.0f));
			M = glm::rotate(M, w * t, glm::vec3(0, 0, 1));
			M = glm::translate(M, glm::vec3(-pTop, 0.0f));
		}
		else if (gState.mode == Mode::Pulse)
		{
			// Escala isótropa alrededor del centroide
			const float A = 0.2f;
			const float w = 2.0f;
			const float s = 1.0f + A * std::sin(w * t);
			M = glm::translate(M, glm::vec3(c, 0.0f));
			M = glm::scale(M, glm::vec3(s, s, 1.0f));
			M = glm::translate(M, glm::vec3(-c, 0.0f));
		}
		else
		{ // RigidCircle
			// Vértice superior describe circunferencia de radio R
			const float R = 0.3f;
			const float w = 0.8f;
			const float xd = R * std::cos(w * t);
			const float yd = R * std::sin(w * t);
			M = glm::translate(M, glm::vec3(xd - pTop.x, yd - pTop.y, 0.0f));
		}

		shader.use();
		shader.setMat4("uModel", &M[0][0]);
		shader.setMat4("uView", &V[0][0]);
		shader.setMat4("uProj", &P[0][0]);
		shader.setVec3("uColor", 0.92f, 0.52f, 0.20f);

		mesh.bind();
		mesh.draw();

		glfwSwapBuffers(win);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
