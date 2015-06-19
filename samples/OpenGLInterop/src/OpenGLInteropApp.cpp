#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"

#include "cinder/gl/Vao.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"

#include "Cinder-OpenCl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma OPENCL EXTENSION cl_khr_gl_event : enable

class OpenGLInteropApp : public App {
  public:
	void setup();
	void update();
	void draw();
	void setupCl();
	void setupGl();
	void performQueries();
	void computeTexture();
	void computeVbo();
	
	static void contextErrorCallback( const char *errinfo,
									 const void *private_info,
									 ::size_t cb,
									 void *user_data)
	{
		cout << "ERROR: " << errinfo << endl;
	}
	
	gl::VaoRef			mGlVao;
	gl::VboRef			mGlVbo;
	gl::GlslProgRef		mGlGlsl;
	gl::Texture2dRef	mGlTexture;
	cl::BufferGL		mClVbo;
	cl::ImageGL			mClImage;
	cl::Platform		mClPlatform;
	cl::Context			mClContext;
	cl::CommandQueue	mClCommandQueue;
	cl::Program			mClProgram;
	cl::Kernel			mVboKernel, mTextureKernel;
};

void OpenGLInteropApp::setup()
{
	setupGl();
	setupCl();
}

void OpenGLInteropApp::setupGl()
{
	mGlVbo = gl::Vbo::create( GL_ARRAY_BUFFER, getWindowHeight() * sizeof(vec4), NULL, GL_STREAM_DRAW );
	
	mGlVao = gl::Vao::create();
	
	{
		gl::ScopedVao scopeVao( mGlVao );
		gl::ScopedBuffer scopeBuffer( mGlVbo );
		
		gl::enableVertexAttribArray(0);
		gl::vertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}
	
	mGlGlsl = gl::GlslProg::create( gl::GlslProg::Format()
										.vertex( loadAsset( "basic.vert" ) )
										.fragment( loadAsset( "basic.frag" ) )
										.attribLocation( "position", 0 )
								   .uniform( gl::UniformSemantic::UNIFORM_MODEL_VIEW_PROJECTION, "mvp" ) );
	mGlTexture = gl::Texture2d::create( getWindowWidth(), getWindowHeight(), gl::Texture2d::Format().internalFormat( GL_RGB32F ) );
	
}

void OpenGLInteropApp::setupCl()
{
	// Get all of the platforms on this system
	std::vector<cl::Platform> platforms;
	cl::Platform::get( &platforms );
	// Assign the platform that we need
	mClPlatform = platforms[0];
	
    // Next, create an OpenCL context on the selected platform.
	// And authorize creation of the sharing context
	// Get the GPU devices from the platform
	std::vector<cl::Device> devices;
	mClPlatform.getDevices( CL_DEVICE_TYPE_GPU, &devices );
	for( auto & device : devices ) {
		cout << "DEVICE NAME: " << device.getInfo<CL_DEVICE_NAME>() << endl;
	}
	// Create an OpenCL context on first available platform
	mClContext = cl::Context( devices,
							 getDefaultSharedGraphicsContextProperties(),
							 &OpenGLInteropApp::contextErrorCallback );
	
    // Create a command-queue on the first device available
    // on the created context
    mClCommandQueue = cl::CommandQueue( mClContext );
	
    // Create program from source
	mClProgram = cl::Program( mClContext, loadString( loadAsset( "GlInterop.cl" ) ), true );
	
    // Create kernel object
	mVboKernel = cl::Kernel( mClProgram, "init_vbo_kernel" );
	mTextureKernel = cl::Kernel( mClProgram, "init_texture_kernel" );
	
	mClVbo = cl::BufferGL( mClContext, CL_MEM_READ_WRITE,  mGlVbo->getId() );
	mClImage = cl::ImageGL( mClContext, CL_MEM_READ_WRITE, mGlTexture->getTarget(), 0, mGlTexture->getId() );
}

void OpenGLInteropApp::computeTexture()
{
	auto size = getWindowSize();
	static cl_int seq = 0;
	seq = ( seq + 1 ) % ( size.x * 2 );
	
    mTextureKernel.setArg( 0, mClImage );
    mTextureKernel.setArg( 1, sizeof(cl_int), &size.x );
    mTextureKernel.setArg( 2, sizeof(cl_int), &size.y );
    mTextureKernel.setArg( 3, sizeof(cl_int), &seq );
	
	std::vector<cl::Memory> glMemory = { mClImage };
	mClCommandQueue.enqueueAcquireGLObjects( &glMemory );
	
	mClCommandQueue.enqueueNDRangeKernel( mTextureKernel,
										 cl::NullRange,
										 cl::NDRange( size.x, size.y ),
										 cl::NDRange( 32, 4 ) );
	mClCommandQueue.enqueueReleaseGLObjects( &glMemory );
}

void OpenGLInteropApp::computeVbo()
{
	static auto size = getWindowSize();
	// a small internal counter for animation
	static cl_int seq = 0;
	seq = (seq+1)%(size.x);

    // Set the kernel arguments, send the cl_mem object for the VBO
    mVboKernel.setArg( 0, mClVbo );
	mVboKernel.setArg( 1, sizeof(cl_int), &size.x );
	mVboKernel.setArg( 2, sizeof(cl_int), &size.y );
	mVboKernel.setArg( 3, sizeof(cl_int), &seq );
	
	
	// Acquire the GL Object
	// Note, we should ensure GL is completed with any commands that might affect this VBO
	// before we issue OpenCL commands
	
	std::vector<cl::Memory> glMemory = { mClVbo };
	mClCommandQueue.enqueueAcquireGLObjects( &glMemory );
	
    // Queue the kernel up for execution across the array
	mClCommandQueue.enqueueNDRangeKernel( mVboKernel,
										 cl::NullRange,
										 cl::NDRange( static_cast<size_t>(size.y) ),
										 cl::NDRange( 32 ) );
	
	// Release the GL Object
	// Note, we should ensure OpenCL is finished with any commands that might affect the VBO
	mClCommandQueue.enqueueReleaseGLObjects( &glMemory );
}

void OpenGLInteropApp::update()
{
	computeTexture();
	computeVbo();
}

void OpenGLInteropApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::setMatricesWindow( getWindowSize() );
	
	gl::draw( mGlTexture );
	gl::ScopedVao scopeVao( mGlVao );
	gl::ScopedGlslProg scope( mGlGlsl );
	
	gl::setDefaultShaderVars();
	
	gl::drawArrays( GL_LINES, 0, getWindowHeight() * 2 );
}

void prepareSettings( App::Settings *settings ) { settings->setWindowSize( 256, 256 ); }

CINDER_APP( OpenGLInteropApp, RendererGl, &prepareSettings )
