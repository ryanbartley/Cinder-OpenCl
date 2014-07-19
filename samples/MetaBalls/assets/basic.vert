#version 330

in vec4 position;
in vec4 color;

uniform mat4 mvp;

out vec4 vColor;

void main() {
	gl_Position = mvp * position;
	vColor = color;
	gl_PointSize = 100.0;
}