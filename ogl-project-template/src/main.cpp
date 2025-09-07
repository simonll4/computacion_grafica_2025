extern "C"{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}

#include <iostream>

static const char* kWindowTitle = "OpenGL template project";
static const int kWindowWidth   = 800;
static const int kWindowHeight  = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
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
	
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	
	while(!glfwWindowShouldClose(window)){
		// input
		processInput(window);
		
		// rendering commands here
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();
	
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	glfwSetWindowShouldClose(window, true);
}

void print_gl_version(void){
    // get version info
     const GLubyte* renderer = glGetString(GL_RENDERER);    // get renderer string
     const GLubyte* version = glGetString(GL_VERSION);      // version as a string

     std::cout << "Renderer: " << renderer << std::endl;
     std::cout << "OpenGL version supported " << version << std::endl;
}
