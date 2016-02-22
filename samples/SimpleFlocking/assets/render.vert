// flocking_render.vs
// OpenGL SuperBible
// Example of using transform feedback to implement flocking
// Program by Graham Sellers.

// Flocking render vertex shader
#version 330

// Model-view-projection matrix
uniform mat4 ciModelViewProjection;

// Instanced attributes (position and velocity of the flock member
in vec4 ciPosition;
in vec4 instPosition;
in vec4 instVelocity;

out vec3 vColor;

// This is essentially a 'lookat' matrix to make the airplane fly fowards
// and hopefully, stay upright
mat3 rotateByDirection( vec3 direction )
{
	mat3 transform = mat3(1);
	if (direction.x == 0 && direction.z == 0)
	{
		if (direction.y < 0){ // rotate 180 degrees
			transform = mat3( vec3(-1.0, 0.0, 0.0), vec3( 0.0, -1.0, 0.0), vec3( 0.0,  0.0, 1.0) );
		}
		// else if direction.y >= 0, leave transform as the identity matrix.
	}
	else
	{
		vec3 y = normalize(direction);
		vec3 z = normalize(cross(y, vec3(0, 1, 0)));
		vec3 x = normalize(cross(y, z));
		transform = mat3(x, y, z);
	}
	
	return transform;
}

void main(void)
{
	
	// Get the forwards matrix
	mat3 rotation = rotateByDirection( instVelocity.rgb );
	vec3 pos = ciPosition.xyz;
	pos = rotation * pos;
	vColor = instVelocity.rgb;
	// Output position
	gl_Position = ciModelViewProjection * vec4(vec3(pos + instPosition.xyz), 1.0);
}