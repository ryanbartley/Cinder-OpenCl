#pragma OPENCL EXTENSION cl_khr_gl_event : enable

__kernel void init_vbo_kernel(__global float4 *vbo, int w, int h, int seq)
{
    int gid = get_global_id(0);
	float4 linepts;
	float f = 1.0f;
	float a = (float)h/4.0f;
	float b = (float)w/2.0f;
	
	linepts.x = float(gid);
	linepts.y = b + a * sin(3.14f*2.0f*((float)gid/(float)w*f + (float)seq/(float)w));
	linepts.z = float(gid) + 1.0f;
	linepts.w = b + a * sin(3.14f*2.0f*((float)(gid+1.0f)/(float)w*f + (float)seq/(float)w));
	
	vbo[gid] = linepts;
}

__kernel void init_texture_kernel(__write_only image2d_t im, int w, int h, int seq )
{
	int2 coord = { get_global_id(0), get_global_id(1) };
	float4 color =  {
		(float)coord.x/(float)w,
		(float)coord.y/(float)h,
		(float)abs(seq-w)/(float)w,
		1.0f};
	write_imagef( im, coord, color );
}