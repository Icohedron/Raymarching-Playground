#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define GLEW_STATIC
#include <gl/glew.h>

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"

GLfloat window_width = 1280;
GLfloat window_height = 720;

bool keys[1024];
bool reset_cursor_offset = true;
double last_cursor_x, last_cursor_y;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

void WindowResizeCallback(GLFWwindow *window, int width, int height) {
	window_width = width;
	window_height = height;
	glViewport(0, 0, width, height);
}

void WindowFocusCallback(GLFWwindow *window, int focused) {
	if (!focused) {
		reset_cursor_offset = true;
	}
}

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (action == GLFW_PRESS) {
		keys[key] = true;
	} else if (action == GLFW_RELEASE) {
		keys[key] = false;
	}
}

void UpdateCameraMovement(GLfloat delta_time) {
	if (keys[GLFW_KEY_W]) {
		camera.Move(FORWARD, delta_time);
	}
	if (keys[GLFW_KEY_S]) {
		camera.Move(BACK, delta_time);
	}
	if (keys[GLFW_KEY_A]) {
		camera.Move(LEFT, delta_time);
	}
	if (keys[GLFW_KEY_D]) {
		camera.Move(RIGHT, delta_time);
	}
	if (keys[GLFW_KEY_SPACE]) {
		camera.Move(UP, delta_time);
	}
	if (keys[GLFW_KEY_LEFT_CONTROL] || keys[GLFW_KEY_LEFT_SHIFT]) {
		camera.Move(DOWN, delta_time);
	}
}

void CursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
	if (reset_cursor_offset) {
		last_cursor_x = xpos;
		last_cursor_y = ypos;
		reset_cursor_offset = false;
	}

	GLfloat xoffset = xpos - last_cursor_x;
	GLfloat yoffset = last_cursor_y - ypos;
	last_cursor_x = xpos;
	last_cursor_y = ypos;

	camera.MouseOrientate(xoffset, yoffset);
}

std::string LoadTextFile(const std::string &file_path) {
	std::string source;
	std::ifstream source_file;

	source_file.exceptions(std::ifstream::badbit);

	try {
		source_file.open(file_path);
		std::stringstream file_stream;
		file_stream << source_file.rdbuf();
		source_file.close();
		source = file_stream.str();
	} catch (std::ifstream::failure e) {
		std::cout << "Failed to load file " << file_path << std::endl;
		return "";
	}

	return source;
}

bool CheckShaderError(GLuint shader, GLenum pname, const std::string &name) {
	GLint success;
	GLchar info_log[512];
	glGetShaderiv(shader, pname, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, info_log);
		std::cout << "Error in shader \"" << name << "\"\n" << info_log << std::endl;
		return true;
	}
	return false;
}

int main() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(window_width, window_height, "Raymarching Playground", NULL, NULL);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetWindowSizeCallback(window, WindowResizeCallback);
	glfwSetWindowFocusCallback(window, WindowFocusCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, CursorPosCallback);

	glfwSwapInterval(1);

	glViewport(0, 0, window_width, window_height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	GLuint vertex_shader;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertex_shader_source = LoadTextFile("shaders/shader.vsh");
	const GLchar *vertex_cstring = vertex_shader_source.c_str();
	glShaderSource(vertex_shader, 1, &vertex_cstring, NULL);
	glCompileShader(vertex_shader);
	CheckShaderError(vertex_shader, GL_COMPILE_STATUS, "Vertex");

	GLuint fragment_shader;
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragment_shader_source = LoadTextFile("shaders/shader.fsh");
	const GLchar *fragment_cstring = fragment_shader_source.c_str();
	glShaderSource(fragment_shader, 1, &fragment_cstring, NULL);
	glCompileShader(fragment_shader);
	CheckShaderError(fragment_shader, GL_COMPILE_STATUS, "Fragment");

	GLint program;
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	GLint success;
	GLchar info_log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, info_log);
		std::cout << "Error in program \"Program\"\n" << info_log << std::endl;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	GLfloat vertices[] = {
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f
	};

	GLuint vao;
	glGenVertexArrays(1, &vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	GLfloat delta_time;
	GLfloat last_frame_time = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		GLfloat current_frame_time = glfwGetTime();
		delta_time = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		glfwPollEvents();
		UpdateCameraMovement(delta_time);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);
		glUniform1f(glGetUniformLocation(program, "time"), current_frame_time);
		glUniform2f(glGetUniformLocation(program, "resolution"), window_width, window_height);
		glUniform3f(glGetUniformLocation(program, "cameraPos"), camera.position.x, camera.position.y, camera.position.z);
		glUniform3f(glGetUniformLocation(program, "cameraForward"), camera.forward.x, camera.forward.y, camera.forward.z);
		glUniform3f(glGetUniformLocation(program, "cameraRight"), camera.right.x, camera.right.y, camera.right.z);
		glUniform3f(glGetUniformLocation(program, "cameraUp"), camera.up.x, camera.up.y, camera.up.z);
		//glUniform3f(glGetUniformLocation(program, "world_up"), camera.world_up.x, camera.world_up.y, camera.world_up.z);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}