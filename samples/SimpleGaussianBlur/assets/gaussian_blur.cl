__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void gaussian_blur( __read_only image2d_t image,
							__constant float * mask,
							__write_only image2d_t blurredImage,
							__private int maskSize
							) {
	
	const int2 pos = { get_global_id(0), get_global_id(1) };
	
	// Collect neighbor values and multiply with gaussian
	float4 sum = 0.0f;
	// Calculate the mask size based on sigma (larger sigma, larger mask)
	for(int a = -maskSize; a < maskSize+1; a++) {
		for(int b = -maskSize; b < maskSize+1; b++) {
			sum += mask[a+maskSize+(b+maskSize)*(maskSize*2+1)]
			*read_imagef(image, sampler, pos + (int2)(a,b));
		}
	}
	
	write_imagef( blurredImage, pos, sum );
}