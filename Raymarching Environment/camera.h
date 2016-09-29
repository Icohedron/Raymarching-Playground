#pragma once

#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Default value constants
const GLfloat kYaw = -90.0f;
const GLfloat kPitch = 0.0f;
const GLfloat kSpeed = 3.0f;
const GLfloat kSensitivity = 0.1f;
const GLfloat kFov = 45.0f;

enum CameraMovementDirection {
	FORWARD,
	BACK,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera {
public:
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	GLfloat pitch;
	GLfloat yaw;

	GLfloat movement_speed;
	GLfloat mouse_sensitivity;

	GLfloat fov;

	Camera(glm::vec3 position = glm::vec3(0.0f), GLfloat yaw = kYaw, GLfloat pitch = kPitch, glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f));
	void MouseOrientate(GLfloat xoffset, GLfloat yoffset);
	void Move(CameraMovementDirection direction, GLfloat delta_time);
private:
	void UpdateCameraVectors();
};

