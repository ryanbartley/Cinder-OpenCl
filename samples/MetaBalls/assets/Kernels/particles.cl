kernel void particle_update(__global float4* positions, 
                            __global float4* velocities, 
                            __global float* lifetimes,  
                            __global float4* randoms, 
                             float max_life,
                             float min_velocity, 
                             float time_difference,
                             int reset, 
                             int random,
                             int particle_count  ) {
  
  const int i = get_global_id(0);
  
  lifetimes[i] = lifetimes[i] + time_difference;
  
  if ((lifetimes[i] > max_life) || ( length(velocities[i]) < min_velocity ) || reset) {
    
    lifetimes[i] = 0.0f;
    
    //positions[i] = (float4)(0,0,0,1);
    positions[i] = (float4)(32,15,32,1);
    
    int random_index = (random + i) % particle_count;
    float rx = randoms[random_index].x;
    float ry = randoms[random_index].y;
    float rz = randoms[random_index].z;
    velocities[i] = (float4)(rx, ry, rz, 0) * 5;
  
  } else {
  
    /* Update positions and velocity */
    positions[i].xyz = positions[i].xyz + (velocities[i].xyz * time_difference);
    velocities[i].y = velocities[i].y - (9.81f * time_difference);
    
    /* Bounce on floors */
    if (positions[i].y < 15.0f) {
      velocities[i].xyz = velocities[i].xyz * 0.75f;
      velocities[i].y = -velocities[i].y;
    }

  }
}