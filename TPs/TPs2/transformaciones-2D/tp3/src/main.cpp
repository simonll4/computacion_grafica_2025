/*
 * TP3 - Movimiento Orbital con Orientación Radial
 * 
 * Este programa implementa un triángulo que orbita alrededor de un punto central
 * manteniendo siempre su orientación "hacia afuera" del centro de la órbita.
 * 
 * FUNCIONAMIENTO:
 * - El vértice superior del triángulo describe una circunferencia
 * - El triángulo se orienta de manera que su eje -Y local apunte hacia el radio
 * - Esto crea el efecto de que el triángulo "mira hacia afuera" durante la órbita
 * 
 * TRANSFORMACIONES APLICADAS:
 * 1. Rotación alrededor del vértice superior: Mrot = T(pTop) * Rz(phi) * T(-pTop)
 * 2. Traslación a la posición orbital: M = T(d(t) - pTop) * Mrot
 * 
 * Donde:
 * - phi = atan2(yd - cy, xd - cx) + PI/2 (orientación radial)
 * - d(t) = center + R * (cos(wt), sin(wt)) (posición orbital)
 * 
 * CONTROLES:
 *   SPACE: Pausar/reanudar animación
 *   ESC:   Salir del programa
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

#include <iostream>
#include <vector>
#include <array>
#include <cmath>

#include "Shader.h"
#include "SimpleMesh.h"
#include "Transform.h"
#include "Ortho.h"

// Configuración de ventana y estado global
static const char *kTitle = "TP3 - Orbita + radial (SPACE pausa)";
static int kW = 1200, kH = 1200;
static glm::mat4 gProj(1.0f); // Cache de matriz de proyección

// Estado de la aplicación para manejo de tiempo y pausas
struct AppState
{
	bool paused = false;     // Estado de pausa
	double t0 = 0.0;         // Tiempo base para cálculos
	double tPause = 0.0;     // Momento en que se pausó
} gState;

// Geometría del triángulo en coordenadas locales
// Vértices del triángulo: v0 es el vértice superior (pivote de rotación)
static const std::array<float, 9> kTri = {
	0.0f, 0.30f, 0.0f,    // v0 = Vértice superior (pivote)
	-0.25f, -0.20f, 0.0f, // v1 = Vértice inferior izquierdo
	0.25f, -0.20f, 0.0f   // v2 = Vértice inferior derecho
};

// Obtiene la posición del vértice superior (pivote)
static inline glm::vec2 topLocal() { return {kTri[0], kTri[1]}; }

// Parámetros de la órbita circular
struct Orbit
{
	glm::vec2 center{0.0f, 0.0f}; // Centro de la órbita
	float R = 0.20f;              // Radio de la órbita
	float w = 1.00f;              // Velocidad angular (rad/s)
} gOrbit;

// Variables para renderizar la órbita como guía visual
static GLuint gOrbitVAO = 0, gOrbitVBO = 0;
static GLsizei gOrbitCount = 0;

// Crea el VAO para dibujar la órbita como una circunferencia
// R: radio de la circunferencia, N: número de puntos
static void makeOrbitVAO(float R, int N = 96)
{
	// Generar puntos de la circunferencia
	std::vector<glm::vec2> pts;
	pts.reserve(N);
	for (int i = 0; i < N; ++i)
	{
		float a = (2.0f * float(M_PI) * i) / float(N);
		pts.push_back({R * std::cos(a), R * std::sin(a)});
	}
	
	// Crear y configurar VAO/VBO
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

// Callbacks y funciones de tiempo
// Callback para redimensionamiento de ventana
static void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
	kW = w;
	kH = h;
	glViewport(0, 0, w, h);
	// Actualizar matriz de proyección manteniendo aspect ratio
	gProj = orthoAspect((float)kW, (float)kH, 0.8f);
}

// Calcula el tiempo transcurrido respetando las pausas
static float tNow()
{
	const double now = glfwGetTime();
	const double t = gState.paused ? (gState.tPause - gState.t0)  // Tiempo congelado
								   : (now - gState.t0);              // Tiempo actual
	return float(t);
}

// Procesa la entrada del teclado
static void processInput(GLFWwindow *win)
{
	// ESC: salir del programa
	if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(win, GLFW_TRUE);

	// SPACE: pausar/reanudar (detección de flanco)
	static bool spacePressed = false;
	int st = glfwGetKey(win, GLFW_KEY_SPACE);
	if (st == GLFW_PRESS && !spacePressed)
	{
		spacePressed = true;
		if (!gState.paused)
		{
			// Pausar: guardar el momento actual
			gState.paused = true;
			gState.tPause = glfwGetTime();
		}
		else
		{
			// Reanudar: ajustar tiempo base para evitar saltos
			gState.paused = false;
			gState.t0 += (glfwGetTime() - gState.tPause);
		}
	}
	if (st == GLFW_RELEASE)
		spacePressed = false;
}

// Función principal
int main()
{
	// Inicialización de GLFW y creación de ventana
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

	// Configuración de recursos gráficos
	std::vector<float> tri(kTri.begin(), kTri.end());
	SimpleMesh triMesh(tri, GL_TRIANGLES);

	// Cargar shaders
	Shader shader("shaders/basic.vert", "shaders/basic.frag");

	// Configuración inicial de OpenGL
	glClearColor(0.08f, 0.08f, 0.10f, 1.0f); // Fondo gris oscuro
	gState.t0 = glfwGetTime();                // Inicializar tiempo
	framebuffer_size_callback(win, kW, kH);   // Configurar proyección inicial

	// Datos constantes para la animación
	const glm::vec2 pTop = topLocal(); // Posición del pivote
	const glm::mat4 V(1.0f);           // Matriz de vista (identidad para 2D)

	// Crear geometría de la órbita para visualización
	makeOrbitVAO(gOrbit.R, 96);

	// Bucle principal de renderizado
	while (!glfwWindowShouldClose(win))
	{
		processInput(win);
		glClear(GL_COLOR_BUFFER_BIT);

		const float t = tNow(); // Tiempo actual de animación

		// Calcular posición orbital del vértice superior
		const glm::vec2 center = gOrbit.center;
		const glm::vec2 d = center + gOrbit.R * glm::vec2(std::cos(gOrbit.w * t), 
															 std::sin(gOrbit.w * t));
		
		// Calcular ángulo de orientación (hacia afuera del centro)
		const float theta = std::atan2(d.y - center.y, d.x - center.x);
		const float phi = theta + float(M_PI_2); // +90° para orientar -Y hacia el radio

		// Construir matriz de transformación compuesta
		// M = T(d - pTop) * [T(pTop) * Rz(phi) * T(-pTop)]
		const glm::mat4 I(1.0f);
		const glm::mat4 T_toTop = glm::translate(I, glm::vec3(pTop, 0.0f));
		const glm::mat4 T_fromTop = glm::translate(I, glm::vec3(-pTop, 0.0f));
		const glm::mat4 R_about = glm::rotate(I, phi, glm::vec3(0, 0, 1));
		const glm::mat4 Mrot = T_toTop * R_about * T_fromTop;  // Rotación sobre pivote
		const glm::mat4 T_move = glm::translate(I, glm::vec3(d - pTop, 0.0f));
		const glm::mat4 M = T_move * Mrot;  // Traslación + rotación

		// Renderizar triángulo
		shader.use();
		shader.setMat4("uProj", &gProj[0][0]);   // Proyección
		shader.setMat4("uView", &V[0][0]);       // Vista
		shader.setMat4("uModel", &M[0][0]);      // Transformación del triángulo
		shader.setVec3("uColor", 0.92f, 0.52f, 0.20f); // Color naranja
		triMesh.bind();
		triMesh.draw();

		// Renderizar órbita como guía visual
		shader.setMat4("uModel", &I[0][0]);      // Sin transformación
		shader.setVec3("uColor", 0.55f, 0.55f, 0.65f); // Color gris
		glBindVertexArray(gOrbitVAO);
		glDrawArrays(GL_LINE_LOOP, 0, gOrbitCount);
		glBindVertexArray(0);

		glfwSwapBuffers(win);
		glfwPollEvents();
	}

	// Limpieza de recursos
	if (gOrbitVBO)
		glDeleteBuffers(1, &gOrbitVBO);
	if (gOrbitVAO)
		glDeleteVertexArrays(1, &gOrbitVAO);

	glfwTerminate();
	return 0;
}