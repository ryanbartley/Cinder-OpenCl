#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "cinder/gl/Vao.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"

#include "Cinder-OpenCl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OpenGLInteropApp : public App {
  public:
	void setup();
	void update();
	void draw();
	
	void setupClContext();
	void setupCl();
	void setupGl();
	void performQueries();
	void computeTexture( const vec2 &size, cl_int seq );
	void computeBuffer( const vec2 &size, cl_int seq );
	
	static void contextErrorCallback( const char *errinfo, const void *private_info,
									 ::size_t cb, void *user_data )
	{
		CI_LOG_E( errinfo );
	}
	
	gl::BatchRef		mGlBatch;
	gl::VboRef			mGlBuffer;
	gl::Texture2dRef	mGlTexture;
	
	ocl::BufferGL		mClBuffer;
	ocl::ImageGL			mClImage;
	ocl::Context			mClContext;
	ocl::CommandQueue	mClCommandQueue;
	ocl::Kernel			mVboKernel, mTextureKernel;
};

void OpenGLInteropApp::setup()
{
	// First, we setup the context that's going to share resources with OpenGL.
	setupClContext();
	// Next, we can create our OpenGL resources.
	setupGl();
	// Finally, we create the resources that depend on OpenGL.
	setupCl();
}

void OpenGLInteropApp::setupClContext()
{
	// This is all the same as within the HelloWorld sample. Except where notated.
	std::vector<ocl::Platform> platforms;
	ocl::Platform::get( &platforms );
	
	std::vector<ocl::Device> devices;
	platforms[0].getDevices( CL_DEVICE_TYPE_GPU, &devices );
	
	// Here's the big difference in setup. When creating the ocl::Context, we want to share
	// recources with the OpenGL Context (Note: that's not the same as the ci::gl::Context).
	// There's an easy helper function within the ci::ocl wrapper called, long-windedly,
	// getDefaultSharedGraphicsContextProperties, which takes a platform. This will set up
	// the properties correctly to share with what Cinder believes your properties are.
	mClContext = ocl::Context( devices,
							 ocl::getDefaultSharedGraphicsContextProperties( platforms[0] ),
							 &OpenGLInteropApp::contextErrorCallback );
	
	mClCommandQueue = ocl::CommandQueue( mClContext );
}

void OpenGLInteropApp::setupGl()
{
	// Create our texture that we're going to share with cl.
	auto texFormat = gl::Texture2d::Format().internalFormat( GL_RGBA32F );
	mGlTexture = gl::Texture2d::create( getWindowWidth(), getWindowHeight(), texFormat );
	
	auto numVertices = getWindowWidth() * 2;
	
	// Create the buffer that we're going to share with cl.
	mGlBuffer = gl::Vbo::create( GL_ARRAY_BUFFER, numVertices * sizeof(vec2), nullptr, GL_STREAM_DRAW );
	// Describe the buffer and create the batch.
	auto attrib = geom::AttribInfo( geom::POSITION, geom::DataType::FLOAT, 2, sizeof(vec2), 0 );
	auto vboMesh = gl::VboMesh::create( numVertices, GL_LINES,
									   { { geom::BufferLayout( { attrib } ), mGlBuffer } } );
	auto glsl = gl::GlslProg::create( gl::GlslProg::Format()
									 .vertex( loadAsset( "basic.vert" ) )
									 .fragment( loadAsset( "basic.frag" ) ) );
	mGlBatch = gl::Batch::create( vboMesh, glsl );
	
}

void OpenGLInteropApp::setupCl()
{
	// Link the gl buffer and the cl buffer.
	mClBuffer = ocl::BufferGL( mClContext, CL_MEM_READ_WRITE,  mGlBuffer->getId() );
	// An image is similar to a buffer, a memory object. But it is akin to Textures.
	// Here we're going to link the gl texture and the cl image.
	mClImage = ocl::ImageGL( mClContext, CL_MEM_READ_WRITE, mGlTexture->getTarget(), 0, mGlTexture->getId() );
	
    // Create program from source. This time we're not going to store it as we only need
	// the kernels. By creating the kernels, we're implicitly storing the program.
	auto program = ocl::createProgram( mClContext, loadAsset( "GlInterop.cl" ) );
	
	auto size = getWindowSize();
	
    // Create kernel objects...
	mVboKernel = ocl::Kernel( program, "init_vbo_kernel" );
	// Similar to uniforms, kernel arguments can be set once if they're not going to
	// change for the lifetime of the kernel. If they are going to change, you'll
	// need to set them each time they change. Which is what we're doing for the 'seq'
	// arg below.
	mVboKernel.setArg( 0, mClBuffer );
	mVboKernel.setArg( 1, sizeof(cl_int), &size.x );
	mVboKernel.setArg( 2, sizeof(cl_int), &size.y );
	
	// Same as before.
	mTextureKernel = ocl::Kernel( program, "init_texture_kernel" );
	mTextureKernel.setArg( 0, mClImage );
	mTextureKernel.setArg( 1, sizeof(cl_int), &size.x );
	mTextureKernel.setArg( 2, sizeof(cl_int), &size.y );
	
}

void OpenGLInteropApp::computeTexture( const vec2 &size, cl_int seq )
{
	// This argument changes every frame. So we're going to need to send it to
	// the kernel every time we're going to process the kernel.
    mTextureKernel.setArg( 3, sizeof(cl_int), &seq );
	
	try {
		// Here's were we queue the execution of the kernel. The offset is null. The
		// Range is the size of the texture or width and height. Because each kernel
		// is going to execute one pixel of the texture.
		mClCommandQueue.enqueueNDRangeKernel( mTextureKernel,
											 ocl::NullRange,
											 ocl::NDRange( size.x, size.y ) );
	}
	catch( const ocl::Error &e ) {
		CI_LOG_E( e.what() << ": Value: " << ocl::errorToString( e.err() ) );
	}
}

void OpenGLInteropApp::computeBuffer( const vec2 &size, cl_int seq )
{
	// This argument changes every frame. So we're going to need to send it to
	// the kernel every time we're going to process the kernel.
	mVboKernel.setArg( 3, sizeof(cl_int), &seq );
	
	try {
		// Here's where we're executing the kernel. The offset is null. The Range is
		// over the total number of vertices divided by two or width. That's because
		// in each kernel, we're going to process two vertices.
		mClCommandQueue.enqueueNDRangeKernel( mVboKernel,
											 ocl::NullRange,
											 ocl::NDRange( size.x ) );
	}
	catch( const ocl::Error &e ) {
		CI_LOG_E( e.what() << ": Value: " << ocl::errorToString( e.err() ) );
	}
}

void OpenGLInteropApp::update()
{
	static auto size = getWindowSize();
	static cl_int seq = 0;
	// process for the next frame.
	seq = ( seq + 1 ) % ( size.x * 2 );
	
	// Below is the major difference between sharing items between OpenGL and OpenCL, and just
	// processing data with OpenCL. You have to acquire and release objects. There are many
	// ways to do this. The easiest way is, if you have cl_khr_gl_sharing as an active extension
	// to just enqueueAcquire and enqueueRelease GL-initialized CL objects. You can read more
	// about this here...
	// https://www.khronos.org/registry/cl/sdk/1.2/docs/man/xhtml/cl_khr_gl_sharing.html
	
	// First we acquire the CL Object that was created by a GL Object. Calling enqueue will
	// implicitly wait for any OpenGL operations being performed on this object.
	std::vector<ocl::Memory> glMemory = { mClBuffer, mClImage };
	mClCommandQueue.enqueueAcquireGLObjects( &glMemory );

	computeTexture( size, seq );
	computeBuffer( size, seq );

	// After we're finished processing the objects, we'll need to release the CL objects linked
	// to OpenGL. Calling enqueue will implicitly wait for any OpenGL operations being performed
	// on this object.
	mClCommandQueue.enqueueReleaseGLObjects( &glMemory );
}

void OpenGLInteropApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::ScopedMatrices scopeMat;
	gl::setMatricesWindow( getWindowSize() );
	// Draw the texture.
	gl::draw( mGlTexture );
	// Draw the line.
	mGlBatch->draw();
}

void prepareSettings( App::Settings *settings ) { settings->setWindowSize( 256, 256 ); }

CINDER_APP( OpenGLInteropApp, RendererGl, &prepareSettings )
