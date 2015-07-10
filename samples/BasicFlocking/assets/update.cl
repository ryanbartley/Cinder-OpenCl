

__kernel void  update( __global float3 *positions,
					  __global float3 *velocities,
					  int		uFlockSize,
					  float		uDamping,
					  float		uZoneRadiusSqrd,
					  float		uRepelStrength,
					  float		uAlignStrength,
					  float		uAttractStrength,
					  float		uMinThresh,
					  float		uMaxThresh,
					  float		uTimeDelta)
{
	const float minSpeed = 0.5;
	const float maxSpeed = 1.0;
	float3 acc = float3( 0.0f );
	float3 newVel;
	float crowded = 1.0;
	
	int index = get_global_id(0);
	
	float3 myPosition = positions[index];
	float3 myVelocity = velocities[index];
	// Apply rules 1 and 2 for my member in the flock (based on all other
	// members)
	for( int i=0; i<uFlockSize; i++ ){
		if( i != index ) {
			//			if( crowded > 10.0 ) break;
			float3 theirPosition	= positions[index];
			
			float3 dir			= myPosition - theirPosition;
			float dist			= length( dir );
			float distSqrd		= dist * dist;
			
			if( distSqrd < uZoneRadiusSqrd - crowded * 0.01f ){
				float percent		= distSqrd/uZoneRadiusSqrd;
				float3 dirNorm		= normalize( dir );
				
				// repulsion
				if( percent < uMinThresh ){
					float F			= ( uMinThresh/percent - 1.0f ) * uRepelStrength;
					acc				+= dirNorm * F * uTimeDelta;
					crowded			+= ( 1.0f - percent ) * 2.0f;
				}
				else if( percent < uMaxThresh )
				{	// alignment
					float3 theirVelocity		= velocities[i];
					float threshDelta		= uMaxThresh - uMinThresh;
					float adjustedPercent	= ( percent - uMinThresh )/threshDelta;
					float F					= ( 1.0f - ( cos( adjustedPercent * 6.28318f ) * -0.5f + 0.5 ) ) * uAlignStrength;
					acc						+= normalize( theirVelocity ) * F * uTimeDelta;
					crowded					+= ( 1.0f - percent ) * 0.5f;
				}
				else
				{	// attraction
					float threshDelta		= 1.0f - uMaxThresh;
					float adjustedPercent	= ( percent - uMaxThresh )/threshDelta;
					float F					= ( 1.0f - ( cos( adjustedPercent * 6.28318f ) * -0.5f + 0.5 ) ) * uAttractStrength;
					acc						-= dirNorm * F * uTimeDelta;
					crowded					+= ( 1.0f - percent ) * 0.25f;
				}
			}
		}
	}
	
	// pull to center
	acc -= myPosition * 0.0015f;
	
	// Update position based on prior velocity and timestep
	float3 outPosition	= myPosition + myVelocity * uTimeDelta;
	
	// Update velocity based on calculated accelleration
	acc			= normalize( acc ) * min( length( acc ), 10.0f );
	newVel		= myVelocity * uDamping + acc * uTimeDelta;
	
	// Hard clamp speed (mag(velocity) to 10 to prevent insanity
	float newMaxSpeed = maxSpeed + crowded * 0.02f;
	float velLen = length( newVel );
	if( velLen > maxSpeed )
		newVel = normalize( newVel ) * newMaxSpeed;
	else if( velLen < minSpeed )
		newVel = normalize( newVel ) * minSpeed;
	
	
	float3 outVelocity = newVel;
	
	positions[index] = outPosition;
	velocities[index] = outVelocity;
	
}