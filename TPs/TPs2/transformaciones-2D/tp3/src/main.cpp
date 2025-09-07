// TP3 — CGyAV (OpenGL core + GLM)
// Triángulo cuyo TOP orbita en circunferencia y el eje local que pasa por el TOP
// se alinea con el radio (triángulo “hacia afuera”).
//
// Composición (GLM column-major):
//   Mrot = T(pTop) * Rz(phi) * T(-pTop)
//   M    = T(d(t) - pTop) * Mrot
//
// phi = atan2(yd - cy, xd - cx) + PI/2   // -Y local → radial (cuerpo hacia afuera)
//
// Teclas:
//   SPACE: pausa/continuar
//   ESC  : salir

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

#include "Shader.h"
#include "SimpleMesh.h"
#include "Transform.h"
#include "Ortho.h"

// ---------------- Config ventana / estado ----------------
static const char *kTitle = "TP3 - Orbita + radial (SPACE pausa)";
static int kW = 1200, kH = 1200;
static glm::mat4 gProj(1.0f); // cache de proyección (se actualiza en el resize)

struct AppState
{
	bool paused = false;
	double t0 = 0.0;	 // tiempo base (inicio o al salir de pausa)
	double tPause = 0.0; // instante en el que se pausó
} gState;

// ---------------- Geometría local (v0 = TOP/pivote) ----------------
static const std::array<float, 9> kTri = {
	0.0f, 0.30f, 0.0f,	  // v0 = TOP (pivote)
	-0.25f, -0.20f, 0.0f, // v1
	0.25f, -0.20f, 0.0f	  // v2
};
static inline glm::vec2 topLocal() { return {kTri[0], kTri[1]}; }

// ---------------- Parámetros órbita ----------------
struct Orbit
{
	glm::vec2 center{0.0f, 0.0f};
	float R = 0.20f; // radio
	float w = 1.00f; // rad/s
} gOrbit;

// ---- Visual de la órbita (mismo Shader) ----
static GLuint gOrbitVAO = 0, gOrbitVBO = 0;
static GLsizei gOrbitCount = 0;

static void makeOrbitVAO(float R, int N = 96)
{
	std::vector<glm::vec2> pts;
	pts.reserve(N);
	for (int i = 0; i < N; ++i)
	{
		float a = (2.0f * float(M_PI) * i) / float(N);
		pts.push_back({R * std::cos(a), R * std::sin(a)});
	}
	glGenVertexArrays(1, &gOrbitVAO);
	glGenBuffers(1, &gOrbitVBO);
	glBindVertexArray(gOrbitVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gOrbitVBO);
	glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec2), pts.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
	glBindVertexArray(0);
	gOrbitCount = (GLsizei)pts.size();
}

// ---------------- Callbacks / tiempo ----------------
static void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
	kW = w;
	kH = h;
	glViewport(0, 0, w, h);
	gProj = orthoAspect((float)kW, (float)kH, 0.8f); // actualizar proyección sólo en resize
}

// tiempo transcurrido (sin escalado): si está en pausa, se “congela”
static float tNow()
{
	const double now = glfwGetTime();
	const double t = gState.paused ? (gState.tPause - gState.t0)
								   : (now - gState.t0);
	return float(t);
}

static void processInput(GLFWwindow *win)
{
	if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(win, GLFW_TRUE);

	// SPACE: pausa (edge detect)
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
			// al reanudar, corremos t0 para que el reloj continuo no salte
			gState.t0 += (glfwGetTime() - gState.tPause);
		}
	}
	if (st == GLFW_RELEASE)
		spacePressed = false;

	// (Eliminado) R: reset tiempo
	// (Eliminado) Q/E: control de velocidad
}

// ---------------- Main ----------------
int main()
{
	// --- Init GLFW/GLAD y ventana ---
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
	glfwSwapInterval(1); // VSync

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "GLAD load failed\n";
		return 1;
	}

	// --- Triángulo (TOP = v0) ---
	std::vector<float> tri(kTri.begin(), kTri.end());
	SimpleMesh triMesh(tri, GL_TRIANGLES);

	// --- Shader único ---
	Shader shader("shaders/basic.vert", "shaders/basic.frag");

	// --- Estado GL y proyección inicial ---
	glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
	gState.t0 = glfwGetTime();
	framebuffer_size_callback(win, kW, kH); // inicializa gProj con el tamaño actual

	// --- Datos constantes ---
	const glm::vec2 pTop = topLocal(); // pivote (coincide con v0 del VBO)
	const glm::mat4 V(1.0f);		   // View identidad (2D)

	// --- Círculo (VAO) con el MISMO shader ---
	makeOrbitVAO(gOrbit.R, 96);

	while (!glfwWindowShouldClose(win))
	{
		processInput(win);
		glClear(GL_COLOR_BUFFER_BIT);

		const float t = tNow();

		// ---- Cinemática del TOP sobre la circunferencia ----
		const glm::vec2 center = gOrbit.center;
		const glm::vec2 d = center +
							gOrbit.R * glm::vec2(std::cos(gOrbit.w * t), std::sin(gOrbit.w * t));
		const float theta = std::atan2(d.y - center.y, d.x - center.x);
		const float phi = theta + float(M_PI_2); // -Y local → radial (cuerpo hacia afuera)

		// ---- Modelo: M = T(d - pTop) * T(pTop) * Rz(phi) * T(-pTop) ----
		const glm::mat4 I(1.0f);
		const glm::mat4 T_toTop = glm::translate(I, glm::vec3(pTop, 0.0f));
		const glm::mat4 T_fromTop = glm::translate(I, glm::vec3(-pTop, 0.0f));
		const glm::mat4 R_about = glm::rotate(I, phi, glm::vec3(0, 0, 1));
		const glm::mat4 Mrot = T_toTop * R_about * T_fromTop;
		const glm::mat4 T_move = glm::translate(I, glm::vec3(d - pTop, 0.0f));
		const glm::mat4 M = T_move * Mrot;

		// ---- Draw triángulo ----
		shader.use();
		shader.setMat4("uProj", &gProj[0][0]);
		shader.setMat4("uView", &V[0][0]);
		shader.setMat4("uModel", &M[0][0]);
		shader.setVec3("uColor", 0.92f, 0.52f, 0.20f);
		triMesh.bind();
		triMesh.draw();

		// ---- Draw órbita (mismo shader, Model=I) ----
		shader.setMat4("uModel", &I[0][0]);
		shader.setVec3("uColor", 0.55f, 0.55f, 0.65f);
		glBindVertexArray(gOrbitVAO);
		glDrawArrays(GL_LINE_LOOP, 0, gOrbitCount);
		glBindVertexArray(0);

		glfwSwapBuffers(win);
		glfwPollEvents();
	}

	// Limpieza VAO/VBO de órbita
	if (gOrbitVBO)
		glDeleteBuffers(1, &gOrbitVBO);
	if (gOrbitVAO)
		glDeleteVertexArrays(1, &gOrbitVAO);

	glfwTerminate();
	return 0;
}