/*All source code is licensed under BSD
 
 Copyright (c) 2010, Daniel Holden All rights reserved.
 
 Corange is licensed under a basic BSD license.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
 Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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