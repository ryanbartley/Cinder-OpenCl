//
//  GaussianRecurrsive.h
//  GaussianBlur
//
//  Created by ryan bartley on 6/23/15.
//
//

#include "Cinder-OpenCL.h"

using GaussianRecursiveRef = std::shared_ptr<class GaussianRecursive>;

class GaussianRecursive {
public:
	
	static GaussianRecursiveRef create();
	
private:
	GaussianRecurssive();
	
	cl::CommandQueue	mCommandQueue;
	cl::Buffer			mOriginal;
	cl::ImageGL			mOutput;
	cl::Kernel			mSimpleRecursive;
	
};
