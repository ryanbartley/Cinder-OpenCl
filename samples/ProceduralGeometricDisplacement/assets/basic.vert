#version 150

in vec4 ciPosition;

uniform mat4 ciModelViewProjection;

void main()
{
	gl_Position = ciModelViewProjection * vec4( ciPosition.xyz, 1.0 );
}