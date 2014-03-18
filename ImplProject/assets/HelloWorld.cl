__kernel void hello_kernel( __global float *a,
						    __global float *b,
						    __global float *result )
{
	int gid = get_global_id(0);
	result[gid] = a[gid] + b[gid];
}