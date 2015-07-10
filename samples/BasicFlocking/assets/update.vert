// flocking_update.vs
// OpenGL SuperBible
// Example of using transform feedback to implement flocking
// Program by Graham Sellers.

// Flocking update vertex shader
#version 330

// Position and velocity inputs
in vec3 myPosition;
in vec3 myVelocity;

// Outputs (via transform feedback)
out vec3 vPosition;
out vec3 vVelocity;

// TBOs containing the position and velocity of other flock members
uniform samplerBuffer uTexPositions;
uniform samplerBuffer uTexVelocities;

// Parameters...
// This has to match the app's view of the world - no default is given here.
uniform int			uFlockSize;
uniform float		uDamping;
uniform float		uZoneRadiusSqrd;
uniform float		uRepelStrength;
uniform float		uAlignStrength;
uniform float		uAttractStrength;
uniform float		uMinThresh;
uniform float		uMaxThresh;
uniform float		uTimeDelta;

const float minSpeed = 0.5;
const float maxSpeed = 1.0;

void main(void)
{
	vec3 acc = vec3( 0.0 );
	vec3 newVel;
	float crowded = 1.0;
	
	// Apply rules 1 and 2 for my member in the flock (based on all other
	// members)
	for( int i=0; i<uFlockSize; i++ ){
		if( i != gl_VertexID ) {
//			if( crowded > 10.0 ) break;
			vec3 theirPosition	= texelFetch( uTexPositions, i ).xyz;
			
			vec3 dir			= myPosition - theirPosition;
			float dist			= length( dir );
			float distSqrd		= dist * dist;
			
			if( distSqrd < uZoneRadiusSqrd - crowded * 0.01 ){
				float percent		= distSqrd/uZoneRadiusSqrd;
				vec3 dirNorm		= normalize( dir );
				
				// repulsion
				if( percent < uMinThresh ){
					float F			= ( uMinThresh/percent - 1.0 ) * uRepelStrength;
					acc				+= dirNorm * F * uTimeDelta;
					crowded			+= ( 1.0 - percent ) * 2.0;
				} 
				else if( percent < uMaxThresh )
				{	// alignment
					vec3 theirVelocity		= texelFetch( uTexVelocities, i ).xyz;
					float threshDelta		= uMaxThresh - uMinThresh;
					float adjustedPercent	= ( percent - uMinThresh )/threshDelta;
					float F					= ( 1.0 - ( cos( adjustedPercent * 6.28318 ) * -0.5 + 0.5 ) ) * uAlignStrength;
					acc						+= normalize( theirVelocity ) * F * uTimeDelta;
					crowded					+= ( 1.0 - percent ) * 0.5;
				} 
				else 
				{	// attraction
					float threshDelta		= 1.0 - uMaxThresh;
					float adjustedPercent	= ( percent - uMaxThresh )/threshDelta;
					float F					= ( 1.0 - ( cos( adjustedPercent * 6.28318 ) * -0.5 + 0.5 ) ) * uAttractStrength;
					acc						-= dirNorm * F * uTimeDelta;
					crowded					+= ( 1.0 - percent ) * 0.25;
				}
			}
		}
	}
	
	// pull to center
	acc -= myPosition * 0.0015;
	
	// Update position based on prior velocity and timestep
	vPosition	= myPosition + myVelocity * uTimeDelta;
	
	// Update velocity based on calculated accelleration
	acc			= normalize( acc ) * min( length( acc ), 10.0 );
	newVel		= myVelocity * uDamping + acc * uTimeDelta;
	
	// Hard clamp speed (mag(velocity) to 10 to prevent insanity
	float newMaxSpeed = maxSpeed + crowded * 0.02;
	float velLen = length( newVel );
	if( velLen > maxSpeed )
		newVel = normalize( newVel ) * newMaxSpeed;
	else if( velLen < minSpeed )
		newVel = normalize( newVel ) * minSpeed;

	
	vVelocity = newVel;
	// Write position (not strictly necessary as we're capturing user defined
	// outputs using transform feedback)
	gl_Position = vec4( myPosition, 1.0 );
}