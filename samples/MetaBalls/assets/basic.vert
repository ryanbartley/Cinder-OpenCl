#version 330

in vec4 ciPosition;

uniform mat4 ciModelViewProjection;

out vec4 vColor;

void main() {
	gl_Position = ciModelViewProjection * vec4(ciPosition.xyz, 1.0);
	vColor = vec4( ciPosition.w, 0.0, 0.0, 1.0 );
	gl_PointSize = 10.0 * clamp(ciPosition.w, .2, 1.0);
}