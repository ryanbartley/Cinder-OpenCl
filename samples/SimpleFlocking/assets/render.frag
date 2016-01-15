#version 330

out vec4 FragColor;

in vec3 vColor;

void main(void)
{
	FragColor = vec4( vColor, 1.0 );
}