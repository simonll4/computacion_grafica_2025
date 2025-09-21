/**

*TP2 - Guía 2: Transformaciones 2D con OpenGL
 * 
 * Este programa implementa tres tipos de animaciones geométricas aplicadas a un triángulo:
 * 
 * MODO 1 (Tecla '1'): ROTACIÓN ALREDEDOR DEL VÉRTICE SUPERIOR
 * 
 * MODO 2 (Tecla '2'): PULSO ALREDEDOR DEL CENTROIDE
 * 
 * MODO 3 (Tecla '3'): TRASLACIÓN RÍGIDA CIRCULAR
 * 
 * CONTROLES:
 *   - Teclas 1/2/3: Cambiar entre modos de animación
 *   - ESPACIO: Pausar/reanudar animación
 *   - ESC: Salir del programa
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
#include <algorithm>

#include "Shader.h"
#include "SimpleMesh.h"
#include "Ortho.h"

// ============================================================================
// CONFIGURACIÓN DE LA VENTANA Y CONSTANTES GLOBALES
// ============================================================================
static const char *kTitle = "TP2 - Guía 2 (GL core) - Simple";
static int kW = 1200, kH = 1200;

/**
 * Cada modo implementa una transformación geométrica diferente:
 * - RotatePivot: Rotación alrededor del vértice superior
 * - Pulse: Escalado oscilatorio desde el centroide
 * - RigidCircle: Traslación circular manteniendo la forma
 */
enum class Mode
{
	RotatePivot = 1,  ///< Modo 1: Rotación sobre vértice superior
	Pulse = 2,        ///< Modo 2: Escalado pulsante desde centroide
	RigidCircle = 3   ///< Modo 3: Traslación circular rígida
};

/**
 * Gestiona el modo de animación actual, el estado de pausa y los tiempos
 * necesarios para calcular correctamente las animaciones.
 */
struct AppState
{
	Mode mode = Mode::RotatePivot;  ///< Modo de animación actual
	bool paused = false;            ///< Estado de pausa de la animación
	double t0 = 0.0;               ///< Tiempo base (inicio o reanudación)
	double tPause = 0.0;           ///< Momento en que se pausó la animación
} gState;

// ============================================================================
// GEOMETRÍA DEL TRIÁNGULO Y FUNCIONES AUXILIARES
// ============================================================================
/**
 * El triángulo está definido con:
 * - Vértice superior (v0): (0.0, 0.3) - Usado como pivote para rotación
 * - Vértice inferior izquierdo (v1): (-0.25, -0.2)
 * - Vértice inferior derecho (v2): (0.25, -0.2)
 * 
 * Formato: [x0, y0, z0, x1, y1, z1, x2, y2, z2]
 */
static const std::array<float, 9> kTri = {
	0.0f, 0.3f, 0.0f,    // v0: Vértice superior (pivote de rotación)
	-0.25f, -0.2f, 0.0f, // v1: Vértice inferior izquierdo
	0.25f, -0.2f, 0.0f   // v2: Vértice inferior derecho
};

/**
 * Obtiene la posición del vértice superior del triángulo
 * return Coordenadas 2D del vértice superior (usado como pivote)
 */
static glm::vec2 topLocal()
{
	return {kTri[0], kTri[1]};
}

/**
 * El centroide se calcula como el promedio de las coordenadas de los tres vértices:
 * centroide = (v0 + v1 + v2) / 3
 */
static glm::vec2 centroid()
{
	return {
		(kTri[0] + kTri[3] + kTri[6]) / 3.0f,  // Promedio de coordenadas X
		(kTri[1] + kTri[4] + kTri[7]) / 3.0f   // Promedio de coordenadas Y
	};
}

// ============================================================================
// CALLBACKS Y FUNCIONES AUXILIARES
// ============================================================================
/**
 * Actualiza el viewport de OpenGL cuando la ventana cambia de tamaño,
 * manteniendo la relación de aspecto correcta.
 */
static void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
	kW = w;
	kH = h;
	glViewport(0, 0, w, h);
}

/**
 * @brief Imprime información del sistema gráfico
 * 
 * Muestra el renderizador y la versión de OpenGL disponible,
 * útil para debugging y verificación del sistema.
 */
static void print_gl()
{
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "OpenGL:   " << glGetString(GL_VERSION) << "\n";
}

/**
 * CONTROLES DISPONIBLES:
 * - ESC: Cierra la aplicación
 * - Tecla '1': Activa modo de rotación alrededor del vértice superior
 * - Tecla '2': Activa modo de pulso (escalado) alrededor del centroide
 * - Tecla '3': Activa modo de traslación circular rígida
 * - ESPACIO: Pausa/reanuda la animación (mantiene el estado del tiempo)
 */
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

/**
 * Esta función maneja correctamente el tiempo durante las pausas:
 * - Si está pausado: devuelve el tiempo en el momento de la pausa
 * - Si no está pausado: devuelve el tiempo actual menos las pausas acumuladas
 * 
 * Esto permite que las animaciones se reanuden exactamente donde se pausaron.
 */
static float tNow()
{
	const double now = glfwGetTime();
	const double t = gState.paused ? (gState.tPause - gState.t0)  // Tiempo congelado en pausa
								   : (now - gState.t0);              // Tiempo actual menos pausas
	return float(t);
}

// ============================================================================
// PROGRAMA PRINCIPAL
// ============================================================================
/**
 * Inicializa OpenGL, crea la ventana, configura los recursos gráficos
 * y ejecuta el bucle principal de renderizado y animación.
 */
int main()
{
	// ====================================================================
	// INICIALIZACIÓN DE GLFW Y CONFIGURACIÓN DE CONTEXTO OPENGL
	// ====================================================================
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

	// ====================================================================
	// CONFIGURACIÓN DE RECURSOS GRÁFICOS
	// ====================================================================
	std::vector<float> tri(kTri.begin(), kTri.end());
	SimpleMesh mesh(tri, /*useIndices*/ false);  // Crear mesh del triángulo

	Shader shader("shaders/basic.vert", "shaders/basic.frag");  // Cargar shaders

	glClearColor(0.08f, 0.08f, 0.10f, 1.0f);  // Color de fondo (gris oscuro)
	gState.t0 = glfwGetTime();  // Inicializar tiempo base

	// Precalcular puntos importantes del triángulo
	const glm::vec2 pTop = topLocal();  // Vértice superior (pivote)
	const glm::vec2 c = centroid();     // Centro geométrico
	const glm::mat4 V(1.0f);            // Matriz de vista (identidad)

	// ====================================================================
	// BUCLE PRINCIPAL DE RENDERIZADO
	// ====================================================================
	while (!glfwWindowShouldClose(win))
	{
		processInput(win);                    // Procesar entrada del teclado
		glClear(GL_COLOR_BUFFER_BIT);         // Limpiar buffer de color

		const float t = tNow();               // Obtener tiempo actual de animación

		// Configurar proyección ortográfica manteniendo aspect ratio
		glm::mat4 P = orthoAspect((float)kW, (float)kH, 0.8f);

		// ====================================================================
		// CÁLCULO DE LA MATRIZ DE TRANSFORMACIÓN SEGÚN EL MODO ACTIVO
		// ====================================================================
		glm::mat4 M(1.0f);  // Matriz identidad como punto de partida
		
		if (gState.mode == Mode::RotatePivot)
		{
			// MODO 1: ROTACIÓN ALREDEDOR DEL VÉRTICE SUPERIOR
			// Fórmula: M = T(pivot) * R(ángulo) * T(-pivot)
			// 1. Trasladar el pivote al origen
			// 2. Aplicar rotación
			// 3. Trasladar de vuelta a la posición original
			const float w = 1.2f; // Velocidad angular en rad/s
			M = glm::translate(M, glm::vec3(pTop, 0.0f));        // T(pivot)
			M = glm::rotate(M, w * t, glm::vec3(0, 0, 1));       // R(wt) en Z
			M = glm::translate(M, glm::vec3(-pTop, 0.0f));       // T(-pivot)
		}
		else if (gState.mode == Mode::Pulse)
		{
			// MODO 2: ESCALADO PULSANTE DESDE EL CENTROIDE
			// Fórmula: M = T(centroide) * S(factor) * T(-centroide)
			// El factor de escala oscila sinusoidalmente
			const float A = 0.2f;  // Amplitud del pulso (±20%)
			const float w = 2.0f;  // Frecuencia del pulso en rad/s
			const float s = 1.0f + A * std::sin(w * t);  // Factor: 0.8 a 1.2
			M = glm::translate(M, glm::vec3(c, 0.0f));           // T(centroide)
			M = glm::scale(M, glm::vec3(s, s, 1.0f));            // S(factor)
			M = glm::translate(M, glm::vec3(-c, 0.0f));          // T(-centroide)
		}
		else // Mode::RigidCircle
		{
			// MODO 3: TRASLACIÓN CIRCULAR RÍGIDA
			// El vértice superior describe una circunferencia de radio R
			// El triángulo mantiene su forma y orientación
			const float R = 0.3f;  // Radio de la circunferencia
			const float w = 0.8f;  // Velocidad angular en rad/s
			const float xd = R * std::cos(w * t);  // Posición X deseada del vértice
			const float yd = R * std::sin(w * t);  // Posición Y deseada del vértice
			// Calcular desplazamiento necesario desde posición actual
			M = glm::translate(M, glm::vec3(xd - pTop.x, yd - pTop.y, 0.0f));
		}

		// ====================================================================
		// RENDERIZADO DEL TRIÁNGULO
		// ====================================================================
		shader.use();                                          // Activar programa shader
		shader.setMat4("uModel", &M[0][0]);                   // Matriz modelo (transformación)
		shader.setMat4("uView", &V[0][0]);                    // Matriz vista
		shader.setMat4("uProj", &P[0][0]);                    // Matriz proyección
		shader.setVec3("uColor", 0.92f, 0.52f, 0.20f);       // Color naranja

		mesh.bind();                                           // Vincular geometría
		mesh.draw();                                           // Dibujar triángulo

		glfwSwapBuffers(win);                                  // Intercambiar buffers
		glfwPollEvents();                                      // Procesar eventos
	}

	// ====================================================================
	// LIMPIEZA Y FINALIZACIÓN
	// ====================================================================
	glfwTerminate();  // Liberar recursos de GLFW
	return 0;         // Salida exitosa
}
