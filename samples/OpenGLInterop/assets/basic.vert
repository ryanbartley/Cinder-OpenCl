#version 330

in vec4 ciPosition;

uniform mat4 ciModelViewProjection;

void main() {
	gl_Position = ciModelViewProjection * ciPosition;
}