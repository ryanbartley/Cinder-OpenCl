__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void gaussian_blur( __read_only image2d_t image,
							__constant float * mask,
							__write_only image2d_t blurredImage,
							__private int maskSize
							) {
	
	const int2 pos = {get_global_id(0), get_global_id(1)};
	
	// Collect neighbor values and multiply with gaussian
	float4 sum = 0.0f;
	// Calculate the mask size based on sigma (larger sigma, larger mask)
	for(int a = -maskSize; a < maskSize+1; a++) {
		for(int b = -maskSize; b < maskSize+1; b++) {
			sum += mask[a+maskSize+(b+maskSize)*(maskSize*2+1)]
			*read_imagef(image, sampler, pos + (int2)(a,b));
		}
	}
	
	write_imagef(blurredImage, pos, sum);
}

// 	simple 1st order recursive filter kernel
//*****************************************************************
//    - processes one image column per thread
//      parameters:
//      uiDataIn - pointer to input data (RGBA image packed into 32-bit integers)
//      uiDataOut - pointer to output data
//      iWidth  - image width
//      iHeight  - image height
//      a  - blur parameter
//*****************************************************************
//__kernel void SimpleRecursiveRGBA(__global const unsigned int* uiDataIn, __global unsigned int* uiDataOut,
//								  int iWidth, int iHeight, float a)
//{
//	// compute X pixel location and check in-bounds
//	unsigned int X = mul24(get_group_id(0), get_local_size(0)) + get_local_id(0);
//	if (X >= iWidth) return;
//	
//	// advance global pointers to correct column for this work item and x position
//	uiDataIn += X;
//	uiDataOut += X;
//	
//	// start forward filter pass
//	float4 yp = rgbaUintToFloat4(*uiDataIn);  // previous output
//	for (int Y = 0; Y < iHeight; Y++)
//	{
//		float4 xc = rgbaUintToFloat4(*uiDataIn);
//		float4 yc = xc + (yp - xc) * (float4)a;
//		*uiDataOut = rgbaFloat4ToUint(yc);
//		yp = yc;
//		uiDataIn += iWidth;     // move to next row
//		uiDataOut += iWidth;    // move to next row
//	}
//	
//	// reset global pointers to point to last element in column for this work item and x position
//	uiDataIn -= iWidth;
//	uiDataOut -= iWidth;
//	
//	// start reverse filter pass: ensures response is symmetrical
//	yp = rgbaUintToFloat4(*uiDataIn);
//	for (int Y = iHeight - 1; Y > -1; Y--)
//	{
//		float4 xc = rgbaUintToFloat4(*uiDataIn);
//		float4 yc = xc + (yp - xc) * (float4)a;
//		*uiDataOut = rgbaFloat4ToUint((rgbaUintToFloat4(*uiDataOut) + yc) * 0.5f);
//		yp = yc;
//		uiDataIn -= iWidth;   // move to previous row
//		uiDataOut -= iWidth;  // move to previous row
//	}
//}