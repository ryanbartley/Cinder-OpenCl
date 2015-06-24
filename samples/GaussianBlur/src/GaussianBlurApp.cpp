#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"

#include "Cinder-OpenCL.h"

#include "GaussianNaive.hpp"

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
		auto currentSigma = gaussianApp->mSigma;
		for( auto profilingEventIt = profilingEvents.begin(); profilingEventIt != profilingEvents.end(); ++profilingEventIt ) {
			if( (*profilingEventIt)() == event ) {
				auto start = profilingEventIt->getProfilingInfo<CL_PROFILING_COMMAND_START>();
				auto end = profilingEventIt->getProfilingInfo<CL_PROFILING_COMMAND_END>();
				auto timeElapsed = double(end - start) / 1000000000.0;
				CI_LOG_I("Current Sigma (" << currentSigma << ") took " << timeElapsed << " seconds to render or " << 1.0 / timeElapsed << " frames/second.");
				gaussianApp->mCurrentOpPerf = timeElapsed;
				profilingEvents.erase( profilingEventIt );
				return;
			}
		}
	}
	
	void setupCl();
	void setupGlTextureClImages();
	void setupAlgorithms();
	void setupParams();
	
	cl::Platform		mPlatform;
	cl::Context			mContext;
	cl::CommandQueue	mCommandQueue;
	cl::Image2D			mClImage;
	cl::ImageGL			mClInteropResult;
	
	GaussianNaiveRef	mNaiveImpl;
	bool				doThings = true;
	
	gl::Texture2dRef	mGlImageResult;
	
	params::InterfaceGlRef mParams;
	
	ivec2				mImageSize;
	float				mSigma;
	float				mCurrentOpPerf;
	bool				mSigmaUpdated, mUseNaive;
	
	std::vector<cl::Event> mProfilingEvents;
};

void GaussianBlurApp::setup()
{
	mCurrentOpPerf = 0.0f;
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
	mParams->addParam( "Cl Operation Performance", &mCurrentOpPerf );
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
						   getDefaultSharedGraphicsContextProperties( mPlatform ),
						   &GaussianBlurApp::contextInfo );
	
	// Create a command-queue on the on the created context and allow for profiling
	mCommandQueue = cl::CommandQueue( mContext, CL_QUEUE_PROFILING_ENABLE );
}

void GaussianBlurApp::setupGlTextureClImages()
{
	try { 
		// Load image
		auto surface = Surface32f::create( loadImage( loadAsset( "lena.jpg" ) ), SurfaceConstraintsDefault(), true );
		mImageSize = surface->getSize();

		getWindow()->setSize( mImageSize );

		std::vector<cl::ImageFormat> imageFormats;
		mContext.getSupportedImageFormats( CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, &imageFormats );

		for( auto & imageFormat : imageFormats ) {
			console() << "Channel Order: " << constantToString( imageFormat.image_channel_order ) << " Channel Data Type: " << constantToString( imageFormat.image_channel_data_type ) << std::endl;
		}

		// Create an OpenCL Image / texture and transfer data to the device
		// This cl image will hold the surface data as constant.
		mClImage = cl::Image2D( mContext,
								CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
								cl::ImageFormat( CL_RGBA, CL_FLOAT ),
								mImageSize.x, mImageSize.y,
								0, (void*) surface->getData() );

		// Allocate an empty texture which will be the placeholder for the result.
		mGlImageResult = gl::Texture2d::create( mImageSize.x, mImageSize.y,
												gl::Texture2d::Format().internalFormat( GL_RGBA8 ) );

		// Create a buffer for the result
		mClInteropResult = cl::ImageGL( mContext,
										CL_MEM_WRITE_ONLY,
										mGlImageResult->getTarget(),
										0, mGlImageResult->getId() );
	}
	catch( const cl::Error &e ) {
		CI_LOG_E( e.what() + errorToString( e.err() ) );
		doThings = false;
	}
}

void GaussianBlurApp::setupAlgorithms()
{
	if( !doThings ) return;
	mNaiveImpl = GaussianNaive::create( mCommandQueue, mClImage, mClInteropResult, mImageSize );
	mNaiveImpl->setup( mContext );
}

void GaussianBlurApp::keyDown( KeyEvent event )
{
	if( !doThings ) return;
	switch ( event.getCode() ) {
		case KeyEvent::KEY_UP: {
			mSigma++;
		}
		break;
		case KeyEvent::KEY_DOWN: {
			mSigma--;
		}
		break;
		default:
		break;
	}
	// reset with new sigma
	if( mUseNaive )
		mNaiveImpl->setupBlurMask( mContext, mSigma );
	else
		mNaiveImpl->setupBlurMask( mContext, mSigma );
	mSigmaUpdated = true;
}

void GaussianBlurApp::update()
{
	if( !doThings ) return;
	if( mSigmaUpdated ) {
		cl::Event perfEvent;
		if( mUseNaive )
			perfEvent = mNaiveImpl->compute();
		else
			perfEvent = mNaiveImpl->compute();
		perfEvent.setCallback( CL_COMPLETE, &GaussianBlurApp::profileEventCallback, this );
		mProfilingEvents.push_back( std::move( perfEvent ) );
		mSigmaUpdated = false;
	}
}

void GaussianBlurApp::draw()
{
	if( !doThings ) return;
	gl::clear( Color( 0, 0, 0 ) );
	gl::setMatricesWindow( getWindowSize(), false );
	gl::draw( mGlImageResult );
	getWindow()->setTitle( to_string( getAverageFps() ) );
}

CINDER_APP( GaussianBlurApp, RendererGl, []( App::Settings * settings ) { settings->setConsoleWindowEnabled(); } )
