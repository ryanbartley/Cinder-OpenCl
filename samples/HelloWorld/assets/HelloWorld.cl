/*
 Sample from page 46 of the OpenCL Programming Guide
 Aaftab Munshi
 Benedict R. Gaster
 Timothy G. Mattson
 James Fung
 Dan Ginsburg
 Portions Copyright (c) 2012
 */

__kernel void hello_kernel(__global const float *a,
						__global const float *b,
						__global float *result)
{
    int gid = get_global_id(0);

    result[gid] = a[gid] + b[gid];
}
