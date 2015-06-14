#version 330

in vec4 ciPosition;
in vec4 ciColor;

uniform mat4 ciModelViewProjection;

out vec4 vColor;

void main() {
	gl_Position = ciModelViewProjection * ciPosition;
	vColor = ciColor;
	gl_PointSize = 10.0;
}