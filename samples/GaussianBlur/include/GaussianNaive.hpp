//
//  GaussianNaive.hpp
//  GaussianBlur
//
//  Created by ryan bartley on 6/20/15.
//
//

#pragma once

#include "Cinder-OpenCL.h"

using GaussianNaiveRef = std::shared_ptr<class GaussianNaive>;

class GaussianNaive {
public:
	
	static GaussianNaiveRef create( cl::CommandQueue command,
							cl::Image2D original,
							cl::ImageGL output,
							glm::ivec2 imageSize )
	{
		return GaussianNaiveRef( new GaussianNaive( command, original, output, imageSize ) );
	}
	
	void setupBlurMask( cl::Context context, float sigma );
	void setup( cl::Context context );
	cl::Event compute();
	int getCurrentMaskSize() { return mCurrentMaskSize; }
	
private:
	GaussianNaive( cl::CommandQueue command,
				  cl::Image2D original,
				  cl::ImageGL output,
				  glm::ivec2 imageSize )
	: mCommandQueue( command ), mOriginal( original ),
	mOutput( output ), mImageSize( imageSize )
	{
	}
	
	cl::CommandQueue	mCommandQueue;
	cl::Image2D			mOriginal;
	cl::ImageGL			mOutput;
	cl::Buffer			mMask;
	cl::Kernel			mNaiveKernel;
	glm::ivec2			mImageSize;
	int					mCurrentMaskSize;
};

inline void GaussianNaive::setupBlurMask( cl::Context context, float sigma )
{
	mCurrentMaskSize = (int)ceil(3.0f*sigma);
	float * mask = new float[(mCurrentMaskSize*2+1)*(mCurrentMaskSize*2+1)];
	float sum = 0.0f;
	for(int a = -mCurrentMaskSize; a < mCurrentMaskSize+1; a++) {
		for(int b = -mCurrentMaskSize; b < mCurrentMaskSize+1; b++) {
			float temp = exp(-((float)(a*a+b*b) / (2*sigma*sigma)));
			sum += temp;
			mask[a+mCurrentMaskSize+(b+mCurrentMaskSize)*(mCurrentMaskSize*2+1)] = temp;
		}
	}
	// Normalize the mask
	for(int i = 0; i < (mCurrentMaskSize*2+1)*(mCurrentMaskSize*2+1); i++)
		mask[i] = mask[i] / sum;
	
	mMask = cl::Buffer( context,
					   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
					   sizeof(float)*(mCurrentMaskSize*2+1)*(mCurrentMaskSize*2+1),
					   mask );
	
	mNaiveKernel.setArg(1, mMask);
	mNaiveKernel.setArg(3, mCurrentMaskSize);
}

inline void GaussianNaive::setup( cl::Context context )
{
	// Open up the file
	auto programString = loadString( ci::app::loadAsset( "gaussian_blur.cl" ) );
	
	// Compile OpenCL code
	cl::Program program = cl::Program( context, programString, true );
	
	// Run Gaussian kernel
	mNaiveKernel = cl::Kernel(program, "gaussian_blur");
	mNaiveKernel.setArg(0, mOriginal);
	mNaiveKernel.setArg(2, mOutput);
	
	setupBlurMask( context, 2.0f );
}

inline cl::Event GaussianNaive::compute()
{
	cl::Event ret;
	std::vector<cl::Memory> memory = { mOutput };
	mCommandQueue.enqueueAcquireGLObjects( &memory );
	mCommandQueue.enqueueNDRangeKernel( mNaiveKernel,
									   cl::NullRange,
									   cl::NDRange( mImageSize.x, mImageSize.y ),
									   cl::NullRange,
									   nullptr,
									   &ret );
	mCommandQueue.enqueueReleaseGLObjects( &memory );
	return ret;
}