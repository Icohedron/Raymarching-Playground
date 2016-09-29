#include "Camera.h"

Camera::Camera(glm::vec3 position, GLfloat yaw, GLfloat pitch, glm::vec3 world_up)
	: position(position),
	yaw(yaw),
	pitch(pitch),
	world_up(world_up),
	forward(glm::vec3(0.0f, 0.0f, -1.0f)),
	movement_speed(kSpeed),
	mouse_sensitivity(kSensitivity),
	fov(kFov) {
	this->UpdateCameraVectors();
}

void Camera::MouseOrientate(GLfloat xoffset, GLfloat yoffset) {
	xoffset *= this->mouse_sensitivity;
	yoffset *= this->mouse_sensitivity;

	this->yaw += xoffset;
	this->pitch += yoffset;

	if (this->pitch < -89.0f) {
		this->pitch = -89.0f;
	} else if (this->pitch > 89.0f) {
		this->pitch = 89.0f;
	}

	if (yaw >= 360.0f) {
		this->yaw -= 360.0f;
	} else if (yaw <= -360.0f) {
		this->yaw += 360.0f;
	}

	this->UpdateCameraVectors();
}

void Camera::Move(CameraMovementDirection direction, GLfloat delta_time) {
	GLfloat speed = this->movement_speed * delta_time;
	switch (direction) {
	case FORWARD:
		this->position = glm::vec3(this->position.x + this->forward.x * speed, this->position.y, this->position.z + this->forward.z * speed);
		break;
	case BACK:
		this->position = glm::vec3(this->position.x - this->forward.x * speed, this->position.y, this->position.z - this->forward.z * speed);
		break;
	case LEFT:
		this->position -= this->right * speed;
		break;
	case RIGHT:
		this->position += this->right * speed;
		break;
	case UP:
		this->position += this->world_up * speed;
		break;
	case DOWN:
		this->position -= this->world_up * speed;
		break;
	default:
		assert(false);
	}
}

void Camera::UpdateCameraVectors() {
	glm::vec3 forward(
		cos(glm::radians(this->pitch)) * cos(glm::radians(this->yaw)),
		sin(glm::radians(this->pitch)),
		cos(glm::radians(this->pitch)) * sin(glm::radians(this->yaw))
	);
	this->forward = glm::normalize(forward);
	this->right = glm::normalize(glm::cross(this->forward, this->world_up));
	this->up = glm::normalize(glm::cross(this->right, this->forward));
}
