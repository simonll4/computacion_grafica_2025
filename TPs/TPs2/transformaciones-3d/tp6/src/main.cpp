/*
 * TP6 - Transformaciones 3D con Rotación Alrededor de un Pivote
 * 
 * Este programa implementa un triángulo que rota en 3D alrededor de su vértice superior
 * 
 * FUNCIONALIDADES:
 * 1. Rotación 3D continua alrededor del vértice superior como pivote
 * 2. Selección de eje de rotación (X, Y, Z)
 * 3. Sistema de pausa/reanudación con compensación de tiempo
 * 4. Renderizado 3D con perspectiva y profundidad
 * 
 * EJES DE ROTACIÓN:
 * - Eje Z: Rotación perpendicular a la pantalla (como en 2D)
 * - Eje X: Rotación horizontal (el triángulo se inclina adelante/atrás)
 * - Eje Y: Rotación vertical (el triángulo se inclina izquierda/derecha)
 * 
 * CONTROLES:
 *   1: Seleccionar eje Z (perpendicular a pantalla)
 *   2: Seleccionar eje X (horizontal)
 *   3: Seleccionar eje Y (vertical)
 *   ESPACIO: Pausar/reanudar rotación
 *   ESC: Salir
 * 
 * COMPILACIÓN:
 *   make
 *   ./build/OGL-program
 */

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
  
  // Configuración de ventana
  static const char* kTitle = "TP2.1 -> Practico 6 (Rotaciones 3D con pivote en vSuperior)";
  static int kW = 900, kH = 900;
  
  // Enumeración para los ejes de rotación disponibles
  enum class Axis { Z = 1, X = 2, Y = 3 };
  
  // Estado global de la aplicación
  struct AppState {
	Axis axis = Axis::Z;     // Eje de rotación actual (inicia en Z)
	bool paused = false;     // Estado de pausa de la animación
	double t0 = 0.0;         // Tiempo de referencia para la animación
	double tPause = 0.0;     // Momento en que se pausó (para compensar tiempo)
  } gState;
  
  // Geometría del triángulo en coordenadas locales (z=0)
  // El vértice superior (v0) actúa como pivote de rotación
  // Tamaño moderado para visualización óptima en perspectiva 3D
  static const std::array<float, 9> kTri = {
	0.0f,   0.35f, 0.0f,   // v0: Vértice superior (PIVOTE de rotación)
   -0.30f, -0.20f, 0.0f,   // v1: Base izquierda
	0.30f, -0.20f, 0.0f    // v2: Base derecha
  };
  
  // Función auxiliar: obtiene las coordenadas 2D del vértice superior (pivote)
  static inline glm::vec2 topLocal() {
	return {kTri[0], kTri[1]}; // Extrae x,y del vértice superior
  }
  
  // Callbacks para manejo de eventos
  // Callback para redimensionamiento de ventana
  static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
	kW = w; kH = h;                    // Actualizar dimensiones globales
	glViewport(0, 0, w, h);            // Ajustar viewport de OpenGL
  }
  
  // Función auxiliar: muestra información del sistema gráfico
  static void print_gl() {
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "OpenGL:   " << glGetString(GL_VERSION)  << "\n";
  }
  
  // Procesamiento de entrada del teclado
  // ESC: salir, 1/2/3: seleccionar eje Z/X/Y, SPACE: pausar/reanudar
  static void processInput(GLFWwindow* win) {
	// Tecla ESC: cerrar aplicación
	if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	  glfwSetWindowShouldClose(win, GLFW_TRUE);
  
	// Teclas 1/2/3: seleccionar eje de rotación
	if (glfwGetKey(win, GLFW_KEY_1) == GLFW_PRESS) gState.axis = Axis::Z; // Eje Z (perpendicular)
	if (glfwGetKey(win, GLFW_KEY_2) == GLFW_PRESS) gState.axis = Axis::X; // Eje X (horizontal)
	if (glfwGetKey(win, GLFW_KEY_3) == GLFW_PRESS) gState.axis = Axis::Y; // Eje Y (vertical)
  
	// Tecla SPACE: pausar/reanudar con detección de flancos
	static bool spacePressed = false;
	int st = glfwGetKey(win, GLFW_KEY_SPACE);
	if (st == GLFW_PRESS && !spacePressed) {
	  spacePressed = true;
	  if (!gState.paused) {
		// Pausar: guardar momento de pausa
		gState.paused = true;
		gState.tPause = glfwGetTime();
	  } else {
		// Reanudar: compensar tiempo pausado ajustando referencia
		gState.paused = false;
		gState.t0 += (glfwGetTime() - gState.tPause);
	  }
	}
	if (st == GLFW_RELEASE) spacePressed = false; // Detectar liberación de tecla
  }
  
  // Función para obtener tiempo "virtual" que respeta las pausas
  // Devuelve tiempo transcurrido desde el inicio, excluyendo períodos de pausa
  static float tNow() {
	const double now = glfwGetTime();
	// Si está pausado: usar tiempo hasta la pausa
	// Si no está pausado: usar tiempo actual menos tiempo de referencia
	const double t = gState.paused ? (gState.tPause - gState.t0)
								   : (now - gState.t0);
	return float(t);
  }
  
  // Función principal del programa
  int main() {
	// Paso 1: Inicialización de GLFW
	if (!glfwInit()) {
	  std::cerr << "GLFW init failed\n"; return 1;
	}
	// Configurar contexto OpenGL 4.6 Core Profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
	// Paso 2: Crear ventana y configurar contexto
	GLFWwindow* win = glfwCreateWindow(kW, kH, kTitle, nullptr, nullptr);
	if (!win) {
	  std::cerr << "Window creation failed\n";
	  glfwTerminate(); return 1;
	}
	glfwMakeContextCurrent(win);                                    // Activar contexto OpenGL
	glfwSetFramebufferSizeCallback(win, framebuffer_size_callback); // Callback de redimensión
	glfwSwapInterval(1);                                            // V-Sync activado
  
	// Paso 3: Inicializar GLAD (cargador de funciones OpenGL)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
	  std::cerr << "GLAD load failed\n"; return 1;
	}
	print_gl(); // Mostrar información del sistema gráfico
  
	// Paso 4: Crear geometría del triángulo
	// Convertir array estático a vector para SimpleMesh
	std::vector<float> tri(kTri.begin(), kTri.end());
	SimpleMesh mesh(tri, /*useIndices*/ false); // Crear mesh sin índices
  
	// Paso 5: Cargar y compilar shaders
	Shader shader("shaders/basic.vert", "shaders/basic.frag");
  
	// Paso 6: Configurar estado de OpenGL
	glEnable(GL_DEPTH_TEST);                // Habilitar test de profundidad para 3D
	glClearColor(0.08f, 0.08f, 0.10f, 1.0f); // Color de fondo (gris oscuro)
  
	// Paso 7: Inicializar sistema de tiempo y pivote
	gState.t0 = glfwGetTime();              // Establecer tiempo de referencia
  
	const glm::vec2 pTop2 = topLocal();     // Obtener coordenadas 2D del pivote
	const glm::vec3 pTop(pTop2, 0.0f);      // Convertir a 3D (z=0)
  
	// Paso 8: Configurar matriz de vista (cámara) 3D
	// Cámara fija posicionada en +Z mirando hacia el origen
	const glm::mat4 V = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 2.2f),        // Posición de la cámara (ojo)
		glm::vec3(0.0f, 0.0f, 0.0f),        // Punto al que mira (centro)
		glm::vec3(0.0f, 1.0f, 0.0f));       // Vector "arriba"
  
	// Paso 9: Bucle principal de renderizado
	while (!glfwWindowShouldClose(win)) {
	  processInput(win);                                      // Procesar entrada del usuario
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Limpiar buffers de color y profundidad
  
	  // Calcular tiempo actual y configurar proyección perspectiva
	  const float t = tNow();                                 // Tiempo virtual (respeta pausas)
	  const float aspect = (kH == 0) ? 1.0f : float(kW) / float(kH); // Relación de aspecto
	  const glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 10.0f); // Proyección perspectiva
  
	  // Paso 10: Construir matriz de modelo con rotación alrededor del pivote
	  // Fórmula: M = T(pivote) * R_eje(ángulo) * T(-pivote)
	  // Esto rota el objeto alrededor del punto pivote, no del origen
	  glm::mat4 M(1.0f);                      // Matriz identidad
	  M = glm::translate(M, pTop);             // 1. Trasladar pivote al origen
  
	  // 2. Aplicar rotación según el eje seleccionado
	  const float w = 1.2f; // Velocidad angular en radianes por segundo
	  switch (gState.axis) {
		case Axis::Z: M = glm::rotate(M, w * t, glm::vec3(0, 0, 1)); break; // Rotación en Z (2D clásica)
		case Axis::X: M = glm::rotate(M, w * t, glm::vec3(1, 0, 0)); break; // Rotación en X (pitch)
		case Axis::Y: M = glm::rotate(M, w * t, glm::vec3(0, 1, 0)); break; // Rotación en Y (yaw)
	  }
  
	  M = glm::translate(M, -pTop);            // 3. Trasladar de vuelta a posición original
  
	  // Paso 11: Configurar shaders y enviar matrices uniformes
	  shader.use();                            // Activar programa de shaders
	  shader.setMat4("uModel", &M[0][0]);      // Matriz de modelo (transformaciones del objeto)
	  shader.setMat4("uView",  &V[0][0]);      // Matriz de vista (posición de cámara)
	  shader.setMat4("uProj",  &P[0][0]);      // Matriz de proyección (perspectiva)
	  shader.setVec3("uColor", 0.92f, 0.52f, 0.20f); // Color del triángulo (naranja)
  
	  // Paso 12: Renderizar geometría
	  mesh.bind();                             // Vincular VAO del triángulo
	  mesh.draw();                             // Dibujar triángulo
  
	  // Paso 13: Intercambiar buffers y procesar eventos
	  glfwSwapBuffers(win);                    // Mostrar frame renderizado (double buffering)
	  glfwPollEvents();                        // Procesar eventos de ventana y entrada
	}
  
	// Paso 14: Limpieza y terminación
	glfwTerminate();                           // Liberar recursos de GLFW
	return 0;
  }
  