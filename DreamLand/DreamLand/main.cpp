#include "glew.h"		// include GL Extension Wrangler

#include "glfw3.h"  // include GLFW helper library

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/constants.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cctype>

using namespace std;

#define M_PI        3.14159265358979323846264338327950288   /* pi */
#define DEG_TO_RAD	M_PI/180.0f

GLFWwindow* window = 0x00;

GLuint shader_program = 0;

GLuint view_matrix_id = 0;
GLuint model_matrix_id = 0;
GLuint proj_matrix_id = 0;

//Last position corrdiantes for mouse
GLfloat lastY;
GLfloat lastX;
GLfloat yOffset; //difference between current and last Y coordinate

				 //Default camera position
glm::vec3 cameraPosition = glm::vec3(0.0, 0.0, 1.0);

//Default rendering mode (line, point, or triangle/fill)
GLint renderingMode = GL_LINE;

///Transformations
glm::mat4 proj_matrix;
glm::mat4 view_matrix;
glm::mat4 model_matrix;

glm::mat4 rotation_matrix;
glm::mat4 scale_matrix;
glm::mat4 translation_matrix;

GLuint VBO, VAO, EBO;

GLfloat point_size = 3.0f;

// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	0.0f,  1.0f, 0.0f,
};

/*
Setup Model, View, and Projection matrices
*/
void setModelViewProjection()
{
	//view matrix for camera
	view_matrix = lookAt(
		cameraPosition,  //camera in world space (eye)
		glm::vec3(0.0f, 0.0f, -100.0f), //looking from -100 on Z axis (center)
		glm::vec3(0.0f, 1.0f, 0.0f)); //camera head is up(+Y axis) (up)

									  //projection matrix for perspective
	proj_matrix = glm::perspective(
		90.0f, //Field Of View Y axis (fovY)
		1.0f / 1.0f, //aspect 
		0.1f, //near (Z axis)
		100.0f); //far (Z axis)

				 //model matrix (M^World = TRS)
	model_matrix = translation_matrix * rotation_matrix * scale_matrix;

}

///Handle the keyboard input
void keyPressed(GLFWwindow *_window, int key, int scancode, int action, int mods) {
	switch (key) {
	case GLFW_KEY_ESCAPE: //Quit application window
		action == GLFW_PRESS;
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;

		/*
		Rendering mode of model object with P, W, and T keys
		*/
	case GLFW_KEY_P: //Render as points
		action == GLFW_PRESS;
		//render code
		renderingMode = GL_POINT;
		glPolygonMode(GL_FRONT_AND_BACK, renderingMode);
		break;

	case GLFW_KEY_W: //Render as lines
		action == GLFW_PRESS;
		//render code
		renderingMode = GL_LINE;
		glPolygonMode(GL_FRONT_AND_BACK, renderingMode);
		break;

	case GLFW_KEY_T: //Render as triangles/Fill
		action == GLFW_PRESS;
		//render code
		renderingMode = GL_FILL;
		glPolygonMode(GL_FRONT_AND_BACK, renderingMode);
		break;

	default: break;
	}
	return;
}



/*
Setup the lastX and lastY to the cursor position
*/
void setMouseCoordinatesForMouseClicked(GLFWwindow *_window, int key, int action, int mods)
{
	if (key == GLFW_MOUSE_BUTTON_LEFT)
	{
		//action == GLFW_PRESS;
		double x, y;
		glfwGetCursorPos(_window, &x, &y);
		lastX = x;
		lastY = y;

	}


}

/*
Left-Mouse button to zoom in/out
The zoom is applied on the Z-axis and is calculated by doing the difference between the current and last Y position of the cursor.
*/
void setMouseCoordinatesForMouseMovement(GLFWwindow *_window, double xPos, double yPos)
{
	if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT)) //check if left button is pressed or not and return its state
	{
		//mouse moves in the Y direction
		yOffset = lastY - yPos;
		lastY = yPos;

		//new camera position along Z axis
		view_matrix = lookAt(
			cameraPosition + glm::vec3(0.0f, 0.0f, yOffset), //eye
			glm::vec3(0.0f, 0.0f, -100.0f), //center
			glm::vec3(0.0f, 1.0f, 0.0f)); //up

										  //update camera position
		cameraPosition = cameraPosition + glm::vec3(0.0f, 0.0f, yOffset);
	}

}

/*
Resize window with new width and height
*/
void resizeWindow(GLFWwindow *_window, int width, int height)
{
	glViewport(0, 0, width, height);

	//adjust perspective for projection matrix
	proj_matrix = glm::perspective(
		90.0f, //fovY (Y axis)
		1.0f / 1.0f, //aspect
		0.1f, //near (Z axis)
		100.0f); //far (Z axis)

}




bool initialize() {
	/// Initialize GL context and O/S window using the GLFW helper library
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	/// Create a window of size 640x480 and with title "Lecture 2: First Triangle"
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	window = glfwCreateWindow(800, 800, "Project DreamLand", NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	glViewport(0, 0, w, h);
	///Register the keyboard callback function: keyPressed(...)
	glfwSetKeyCallback(window, keyPressed);

	glfwSetCursorPosCallback(window, setMouseCoordinatesForMouseMovement);
	glfwSetMouseButtonCallback(window, setMouseCoordinatesForMouseClicked);
	glfwSetWindowSizeCallback(window, resizeWindow);
	glfwMakeContextCurrent(window);

	/// Initialize GLEW extension handler
	glewExperimental = GL_TRUE;	///Needed to get the latest version of OpenGL
	glewInit();

	/// Get the current OpenGL version
	const GLubyte* renderer = glGetString(GL_RENDERER); /// Get renderer string
	const GLubyte* version = glGetString(GL_VERSION); /// Version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	/// Enable the depth test i.e. draw a pixel if it's closer to the viewer
	glEnable(GL_DEPTH_TEST); /// Enable depth-testing
	glDepthFunc(GL_LESS);	/// The type of testing i.e. a smaller value as "closer"

	return true;
}

bool cleanUp() {
	glDisableVertexAttribArray(0);
	//Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// Close GL context and any other GLFW resources
	glfwTerminate();

	return true;
}

GLuint loadShaders(std::string vertex_shader_path, std::string fragment_shader_path) {
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_shader_path.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_shader_path.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glBindAttribLocation(ProgramID, 0, "in_Position");

	//appearing in the vertex shader.
	glBindAttribLocation(ProgramID, 1, "in_Color");

	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	//The three variables below hold the id of each of the variables in the shader
	//If you read the vertex shader file you'll see that the same variable names are used.
	view_matrix_id = glGetUniformLocation(ProgramID, "view_matrix");
	model_matrix_id = glGetUniformLocation(ProgramID, "model_matrix");
	proj_matrix_id = glGetUniformLocation(ProgramID, "proj_matrix");

	return ProgramID;
}


int main() {

	setModelViewProjection();

	initialize();

	///Load the shaders
	shader_program = loadShaders("DreamLand.vs", "DreamLand.fs");

	// This will identify our vertex buffer
	//GLuint VBO;

	glGenVertexArrays(1, &VAO);
	// Generate 1 buffer, put the resulting identifier in VBO
	glGenBuffers(1, &VBO);


	glBindVertexArray(VAO);
	GLuint EBO;
	glGenBuffers(1, &EBO);

	// The following commands will talk about our 'VBO' buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	//position attribute
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride (next attribute appears every 0 float)
		(void*)0            // array buffer offset
		);
	glEnableVertexAttribArray(0);

	//default rendering
	glPolygonMode(GL_FRONT_AND_BACK, renderingMode);

	while (!glfwWindowShouldClose(window)) {
		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
		glPointSize(point_size);

		glUseProgram(shader_program);

		//Model in World space = TRS (scale first, then rotate, and last translate)
		model_matrix = scale_matrix * rotation_matrix * translation_matrix;

		//Pass the values of the three matrices to the shaders
		glUniformMatrix4fv(proj_matrix_id, 1, GL_FALSE, glm::value_ptr(proj_matrix));
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(model_matrix));

		glBindVertexArray(VAO);
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle

										  //unbind VAO
		glBindVertexArray(0);

		// update other events like input handling
		glfwPollEvents();
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
	}

	cleanUp();
	return 0;
}