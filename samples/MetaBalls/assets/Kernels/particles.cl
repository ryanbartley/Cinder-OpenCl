struct Particle {
	float4 pos;
	float4 vel;
	float4 rand_life;
};

__kernel void particle_update(__global struct Particle *particles,
                             float maxLife,
                             float minVelSqd,
                             float timeDelta,
                             int random,
                             int totalParticles )
{
  
	const int i = get_global_id(0);
	
	struct Particle myPart = particles[i];
	myPart.rand_life.w += timeDelta;
	
	float3 vel = myPart.vel.xyz;
	float lengthSqd = vel.x * vel.x + vel.y * vel.y + vel.z * vel.z;
	
	if ((myPart.rand_life.w > maxLife) || ( lengthSqd < minVelSqd ) ) {
		int random_index = (random + i) % totalParticles;
		float3 randomVals = particles[random_index].rand_life.xyz;
		myPart.vel = (float4)(randomVals, 0) * 5;
		myPart.rand_life.w = 0.0f;
		myPart.pos = (float4)(0,0,0,1);
	}
	else {
		/* Update positions and velocity */
		myPart.pos.xyz = myPart.pos.xyz + ( myPart.vel.xyz * timeDelta );
		myPart.vel.y = myPart.vel.y - (9.81f * timeDelta);
		
		/* Bounce on floors */
		if ( myPart.pos.y <= 0.0f) {
			myPart.vel.xyz *= 0.75f;
			myPart.vel.y = -myPart.vel.y;
		}
	}
	
	particles[i] = myPart;
}