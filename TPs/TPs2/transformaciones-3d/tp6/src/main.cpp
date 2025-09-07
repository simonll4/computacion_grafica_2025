// main.cpp — TP2 Guía 2.1 -> Práctico 6 (Transformaciones 3D)
// Rotación del triángulo alrededor de un eje que pasa por el VÉRTICE SUPERIOR.
// Ejes disponibles: Z (perpendicular a la pantalla), X (horizontal), Y (vertical).

extern "C" {
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
  
  // ---------------- Config ventana ----------------
  static const char* kTitle = "TP2.1 -> Practico 6 (Rotaciones 3D con pivote en vSuperior)";
  static int kW = 900, kH = 900;
  
  // ---------------- Modo / Estado ----------------
  enum class Axis { Z = 1, X = 2, Y = 3 };
  
  struct AppState {
	Axis axis = Axis::Z;
	bool paused = false;
	double t0 = 0.0;     // referencia de tiempo activa
	double tPause = 0.0; // instante en que se pausó
  } gState;
  
  // ---------------- Triángulo (coordenadas locales z=0) ----------------
  // v0 = TOP (pivote). Mantener un tamaño moderado para que la perspectiva lo muestre bien.
  static const std::array<float, 9> kTri = {
	0.0f,   0.35f, 0.0f,   // v0: TOP (pivote)
   -0.30f, -0.20f, 0.0f,   // v1
	0.30f, -0.20f, 0.0f    // v2
  };
  
  static inline glm::vec2 topLocal() {
	return {kTri[0], kTri[1]};
  }
  
  // ---------------- Callbacks ----------------
  static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
	kW = w; kH = h;
	glViewport(0, 0, w, h);
  }
  
  static void print_gl() {
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "OpenGL:   " << glGetString(GL_VERSION)  << "\n";
  }
  
  // ESC, 1/2/3 para elegir eje, SPACE pausa
  static void processInput(GLFWwindow* win) {
	if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	  glfwSetWindowShouldClose(win, GLFW_TRUE);
  
	if (glfwGetKey(win, GLFW_KEY_1) == GLFW_PRESS) gState.axis = Axis::Z;
	if (glfwGetKey(win, GLFW_KEY_2) == GLFW_PRESS) gState.axis = Axis::X;
	if (glfwGetKey(win, GLFW_KEY_3) == GLFW_PRESS) gState.axis = Axis::Y;
  
	static bool spacePressed = false;
	int st = glfwGetKey(win, GLFW_KEY_SPACE);
	if (st == GLFW_PRESS && !spacePressed) {
	  spacePressed = true;
	  if (!gState.paused) {
		gState.paused = true;
		gState.tPause = glfwGetTime();
	  } else {
		gState.paused = false;
		gState.t0 += (glfwGetTime() - gState.tPause); // compensar tiempo pausado
	  }
	}
	if (st == GLFW_RELEASE) spacePressed = false;
  }
  
  // Tiempo “virtual” con pausa
  static float tNow() {
	const double now = glfwGetTime();
	const double t = gState.paused ? (gState.tPause - gState.t0)
								   : (now - gState.t0);
	return float(t);
  }
  
  // ---------------- Programa principal ----------------
  int main() {
	if (!glfwInit()) {
	  std::cerr << "GLFW init failed\n"; return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
	GLFWwindow* win = glfwCreateWindow(kW, kH, kTitle, nullptr, nullptr);
	if (!win) {
	  std::cerr << "Window creation failed\n";
	  glfwTerminate(); return 1;
	}
	glfwMakeContextCurrent(win);
	glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
	glfwSwapInterval(1);
  
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
	  std::cerr << "GLAD load failed\n"; return 1;
	}
	print_gl();
  
	// Un solo triángulo
	std::vector<float> tri(kTri.begin(), kTri.end());
	SimpleMesh mesh(tri, /*useIndices*/ false);
  
	Shader shader("shaders/basic.vert", "shaders/basic.frag");
  
	glEnable(GL_DEPTH_TEST);                // por si luego agregás más geometría
	glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
  
	gState.t0 = glfwGetTime();
  
	const glm::vec2 pTop2 = topLocal();     // pivote en espacio local (x,y)
	const glm::vec3 pTop(pTop2, 0.0f);      // en 3D
  
	// Vista y proyección en 3D:
	// Cámara fija mirando al origen desde +Z.
	const glm::mat4 V = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 2.2f),        // ojo
		glm::vec3(0.0f, 0.0f, 0.0f),        // centro
		glm::vec3(0.0f, 1.0f, 0.0f));       // up
  
	while (!glfwWindowShouldClose(win)) {
	  processInput(win);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
	  const float t = tNow();
	  const float aspect = (kH == 0) ? 1.0f : float(kW) / float(kH);
	  const glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 10.0f);
  
	  // ---------------- Model: T(pivote) * R_axis(w t) * T(-pivote)
	  glm::mat4 M(1.0f);
	  M = glm::translate(M, pTop);
  
	  const float w = 1.2f; // rad/s
	  switch (gState.axis) {
		case Axis::Z: M = glm::rotate(M, w * t, glm::vec3(0, 0, 1)); break;
		case Axis::X: M = glm::rotate(M, w * t, glm::vec3(1, 0, 0)); break;
		case Axis::Y: M = glm::rotate(M, w * t, glm::vec3(0, 1, 0)); break;
	  }
  
	  M = glm::translate(M, -pTop);
  
	  shader.use();
	  shader.setMat4("uModel", &M[0][0]);
	  shader.setMat4("uView",  &V[0][0]);
	  shader.setMat4("uProj",  &P[0][0]);
	  shader.setVec3("uColor", 0.92f, 0.52f, 0.20f);
  
	  mesh.bind();
	  mesh.draw();
  
	  glfwSwapBuffers(win);
	  glfwPollEvents();
	}
  
	glfwTerminate();
	return 0;
  }
  