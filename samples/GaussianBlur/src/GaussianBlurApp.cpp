#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"
#include "cinder/ip/Blur.h"

#include "Cinder-OpenCL.h"

#include "GaussianNaive.hpp"

#include "Timer.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma OPENCL EXTENSION cl_khr_gl_event : enable

class GaussianBlurApp : public App {
  public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
	
	static void contextInfo( const char *info, const void *private_info, ::size_t cb, void *user_data) { CI_LOG_E( info ); }
	static void profileEventCallback( cl_event event, cl_int event_command_exec_status, void *user_data )
	{
		auto gaussianApp = static_cast<GaussianBlurApp*>(user_data);
		auto & profilingEvents = gaussianApp->mProfilingEvents;
		for( auto profilingEventIt = profilingEvents.begin(); profilingEventIt != profilingEvents.end(); ++profilingEventIt ) {
			if( (*profilingEventIt)() == event ) {
				auto start = profilingEventIt->getProfilingInfo<CL_PROFILING_COMMAND_START>();
				auto end = profilingEventIt->getProfilingInfo<CL_PROFILING_COMMAND_END>();
				auto timeElapsed = double(end - start) / 1000000000.0;
				gaussianApp->mNaiveTime = timeElapsed;
				profilingEvents.erase( profilingEventIt );
				return;
			}
		}
	}
	
	void setupCl();
	void setupGlTextureClImages();
	void setupAlgorithms();
	void setupParams();
	void changeSigma();
	
	cl::Platform		mPlatform;
	cl::Context			mContext;
	cl::CommandQueue	mCommandQueue;
	cl::Image2D			mClImage;
	cl::ImageGL			mClInteropResult;
	
	GaussianNaiveRef	mNaiveImpl;
	
	gl::Texture2dRef	mNaiveResult, mCPUResult;
	ci::Surface8uRef	mOriginalSurface,
						mResultSurface;
	
	params::InterfaceGlRef mParams;
	
	ivec2				mImageSize;
	int					mRadius;
	float				mSigma;
	bool				mSigmaUpdated, mUseNaive;
	float				mNaiveTime, mCpuTime;
	
	std::vector<cl::Event> mProfilingEvents;
};

void GaussianBlurApp::setup()
{
	mNaiveTime = mCpuTime = 0.0f;
	mSigma = 2.0f;
	mSigmaUpdated = true;
	
	
	setupCl();
	setupGlTextureClImages();
	setupAlgorithms();
}

void GaussianBlurApp::setupParams()
{
	mParams = params::InterfaceGl::create( "Gaussian Blur", ivec2( 400, 400 ) );
	mParams->addParam( "Sigma", &mSigma ).updateFn( [&](){ mSigmaUpdated = true; } );
}

void GaussianBlurApp::setupCl()
{
	// Get all of the platforms on this system
	std::vector<cl::Platform> platforms;
	cl::Platform::get( &platforms );
	// Assign the platform that we need
	mPlatform = platforms[0];
	
	// Get the GPU devices from the platform
	std::vector<cl::Device> devices;
	mPlatform.getDevices( CL_DEVICE_TYPE_GPU, &devices );
	// Create an OpenCL context on first available platform
	mContext = cl::Context( devices[0],
						   getDefaultSharedGraphicsContextProperties(),
						   &GaussianBlurApp::contextInfo );
	
	// Create a command-queue on the on the created context and allow for profiling
	mCommandQueue = cl::CommandQueue( mContext, CL_QUEUE_PROFILING_ENABLE );
}

void GaussianBlurApp::setupGlTextureClImages()
{
	// Load image
	mOriginalSurface = Surface8u::create( loadImage( loadAsset( "sunset.jpg" ) ) );
	mImageSize = mOriginalSurface->getSize();
	
	getWindow()->setSize( vec2( mImageSize.x * 2, mImageSize.y ) );
	
	// Create an OpenCL Image / texture and transfer data to the device
	// This cl image will hold the surface data as constant.
	mClImage = cl::Image2D( mContext,
						   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
						   cl::ImageFormat(CL_RGB, CL_UNORM_INT8),
						   mImageSize.x, mImageSize.y,
						   0, (void*)mOriginalSurface->getData());
	
	// Allocate an empty texture which will be the placeholder for the result.
	mNaiveResult = gl::Texture2d::create( mImageSize.x, mImageSize.y );
	
	// Create a buffer for the result
	mClInteropResult = cl::ImageGL( mContext,
								   CL_MEM_WRITE_ONLY,
								   mNaiveResult->getTarget(),
								   0, mNaiveResult->getId() );
	
	mResultSurface = ci::Surface8uRef( new ci::Surface8u( mOriginalSurface->clone() ) );
	
	mCPUResult = gl::Texture2d::create( mImageSize.x, mImageSize.y );
}

void GaussianBlurApp::setupAlgorithms()
{
	mNaiveImpl = GaussianNaive::create( mCommandQueue, mClImage, mClInteropResult, mImageSize );
	mNaiveImpl->setup( mContext );
	
	mRadius = mNaiveImpl->getCurrentMaskSize();
}

void GaussianBlurApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() ) {
		case KeyEvent::KEY_UP: {
			mSigma++;
			changeSigma();
		}
		break;
		case KeyEvent::KEY_DOWN: {
			mSigma--;
			changeSigma();
		}
		break;
		default:
		break;
	}
}

void GaussianBlurApp::changeSigma()
{
	// reset with new sigma
	if( mUseNaive )
		mNaiveImpl->setupBlurMask( mContext, mSigma );
	else
		mNaiveImpl->setupBlurMask( mContext, mSigma );
	
	mResultSurface = ci::Surface8uRef( new ci::Surface8u( mOriginalSurface->clone() ) );
	mSigmaUpdated = true;
	mRadius = mNaiveImpl->getCurrentMaskSize();
}

void GaussianBlurApp::update()
{
	if( mSigmaUpdated ) {
		cl::Event perfEvent;
		if( mUseNaive )
			perfEvent = mNaiveImpl->compute();
		else
			perfEvent = mNaiveImpl->compute();
		perfEvent.setCallback( CL_COMPLETE, &GaussianBlurApp::profileEventCallback, this );
		mProfilingEvents.push_back( std::move( perfEvent ) );
		mSigmaUpdated = false;
		
		ci::Timer cpuTimer;
		cpuTimer.start();
		ip::stackBlur( mResultSurface.get(), mRadius );
		cpuTimer.stop();
		mCpuTime = cpuTimer.getSeconds();
		mCPUResult->update( *mResultSurface );
	}
}

void GaussianBlurApp::draw()
{
	auto font = Font( Font::getDefault().getName(), 20 );
	gl::clear( Color( 0, 0, 0 ) );
	gl::setMatricesWindow( getWindowSize(), true );
	{
		gl::ScopedModelMatrix scopeModel;
		gl::translate( mNaiveResult->getSize() / 2 );
		gl::rotate( toRadians( 180.0f ) );
		gl::translate( - mNaiveResult->getSize() / 2 );
		gl::draw( mNaiveResult );
	}
	{
		gl::ScopedBlendAlpha blend;
		auto string = "Naive Impl: " + to_string( 1.0f / mNaiveTime ) + " fps";
		gl::drawString( string, vec2( 50, 50 ), ColorA( 1, 1, 1, 1 ), font );
	}
	gl::translate( vec2( getWindowWidth() / 2, 0 ) );
	gl::draw( mCPUResult );
	{
		gl::ScopedBlendAlpha blend;
		auto string = "CPU Impl: " + to_string( 1.0f / mCpuTime ) + " fps";
		gl::drawString( string, vec2( 50, 50 ), ColorA( 1, 1, 1, 1 ), font );
	}
	getWindow()->setTitle( to_string( getAverageFps() ) );
}

CINDER_APP( GaussianBlurApp, RendererGl )
