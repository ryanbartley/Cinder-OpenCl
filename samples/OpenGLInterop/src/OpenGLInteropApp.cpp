#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/Vao.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"

#include "BufferObj.h"
#include "Platform.h"
#include "Device.h"
#include "Context.h"
#include "Program.h"
#include "CommandQueue.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OpenGLInteropApp : public App {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void setupCl();
	void setupGl();
	void performQueries();
	void computeTexture();
	void computeVbo();
	
	gl::VaoRef			mGlVao;
	gl::VboRef			mGlVbo;
	gl::GlslProgRef		mGlGlsl;
	cl::BufferObjRef	mClVbo;
	cl::PlatformRef		mClPlatform;
	cl::ContextRef		mClContext;
	cl::CommandQueueRef	mClCommandQueue;
	cl::ProgramRef		mClProgram;
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
	
	mGlVao->bind();
	mGlVbo->bind();
	
	gl::enableVertexAttribArray(0);
	gl::vertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	mGlGlsl = gl::GlslProg::create( gl::GlslProg::Format()
										.vertex( loadAsset( "basic.vert" ) )
										.fragment( loadAsset( "basic.frag" ) )
										.attribLocation( "position", 0 )
								   .uniform( gl::UniformSemantic::UNIFORM_MODEL_VIEW_PROJECTION, "mvp" ) );
	//	initTexture(imWidth,imHeight);
}

void OpenGLInteropApp::setupCl()
{
    // First, select an OpenCL platform to run on.
	auto platforms = cl::Platform::getAvailablePlatforms();
	
	// Iterate through the list of platforms until we find one that supports
	// a GPU device, otherwise fail with an error.
	mClPlatform = cl::Platform::create( platforms[0], CL_DEVICE_TYPE_GPU );
	
    // Next, create an OpenCL context on the selected platform.
	// And authorize creation of the sharing context
    mClContext = cl::Context::create( mClPlatform, true );
	
    // Create a command-queue on the first device available
    // on the created context
    mClCommandQueue = cl::CommandQueue::create( mClPlatform->getDevices()[0] );
	
    // Create program from source
	mClProgram = cl::Program::create( loadAsset( "GlInterop.cl" ) );
	
    // Create kernel object
	mClProgram->createKernel( "init_vbo_kernel" );
	
//	tex_kernel = clCreateKernel(program, "init_texture_kernel", NULL);
//    if (tex_kernel == NULL)
//    {
//        std::cerr << "Failed to create kernel" << std::endl;
//        Cleanup();
//        return 1;
//    }
	
    // Create memory objects that will be used as arguments to
    // kernel
//    if (!CreateMemObjects(context, tex, vbo, &cl_vbo_mem, &cl_tex_mem))
//    {
//        Cleanup();
//        return 1;
//    }
	
	mClVbo = cl::BufferObj::create( mGlVbo, CL_MEM_READ_WRITE );
	
	// Perform some queries to get information about the OpenGL objects
	performQueries();
}

void OpenGLInteropApp::performQueries() {
//	cl_int errNum;
//	std::cout << "Performing queries on OpenGL objects:" << std::endl;
//	// example usage of getting information about a GL memory object
//	cl_gl_object_type obj_type;
//	GLuint objname;
////	errNum = clGetGLObjectInfo( cl_tex_mem,  &obj_type, &objname );
//	if( errNum != CL_SUCCESS ) {
//		std::cerr << "Failed to get object information" << std::endl;
//	} else {
//		if( obj_type == CL_GL_OBJECT_TEXTURE2D ) {
//			std::cout << "Queried a texture object succesfully." << std::endl;
//			std::cout << "Object name is: " << objname << std::endl;
//		}
//		
//	}
//	
//	// Example usage of how to get information about the texture object
//	GLenum param;
//	size_t param_ret_size;
//	errNum = clGetGLTextureInfo( cl_tex_mem, CL_GL_TEXTURE_TARGET, sizeof( GLenum ), &param, &param_ret_size );
//	if( errNum != CL_SUCCESS ) {
//		std::cerr << "Failed to get texture information" << std::endl;
//	} else {
//		// we have set it to use GL_TEXTURE_RECTANGLE_ARB.  We expect it to be reflectedin the query here
//		if( param == GL_TEXTURE_RECTANGLE_ARB ) {
//			std::cout << "Texture rectangle ARB is being used." << std::endl;
//		}
//	}
}

void OpenGLInteropApp::computeTexture()
{
//	cl_int errNum;
//	
//	static cl_int seq =0;
//	seq = (seq+1)%(imWidth*2);
//	
//    errNum = clSetKernelArg(tex_kernel, 0, sizeof(cl_mem), &cl_tex_mem);
//    errNum = clSetKernelArg(tex_kernel, 1, sizeof(cl_int), &imWidth);
//    errNum = clSetKernelArg(tex_kernel, 2, sizeof(cl_int), &imHeight);
//    errNum = clSetKernelArg(tex_kernel, 3, sizeof(cl_int), &seq);
//	
//	size_t tex_globalWorkSize[2] = { imWidth, imHeight };
//	size_t tex_localWorkSize[2] = { 32, 4 } ;
//	
//	glFinish();
//	errNum = clEnqueueAcquireGLObjects(commandQueue, 1, &cl_tex_mem, 0, NULL, NULL );
//	
//    errNum = clEnqueueNDRangeKernel(commandQueue, tex_kernel, 2, NULL,
//                                    tex_globalWorkSize, tex_localWorkSize,
//                                    0, NULL, NULL);
//    if (errNum != CL_SUCCESS)
//    {
//        std::cerr << "Error queuing kernel for execution." << std::endl;
//    }
//	errNum = clEnqueueReleaseGLObjects(commandQueue, 1, &cl_tex_mem, 0, NULL, NULL );
//	clFinish(commandQueue);
//	return 0;
}

void OpenGLInteropApp::computeVbo()
{
	cl_int errNum;
	static auto size = getWindowSize();
	// a small internal counter for animation
	static cl_int seq = 0;
	seq = (seq+1)%(size.x);

    // Set the kernel arguments, send the cl_mem object for the VBO
	auto kernel = mClProgram->getKernelByName("init_vbo_kernel");
	
    kernel->setKernelArg( 0, mClVbo );
	kernel->setKernelArg( 1, sizeof(cl_int), &size.x );
	kernel->setKernelArg( 2, sizeof(cl_int), &size.y );
	kernel->setKernelArg( 3, sizeof(cl_int), &seq );
	
    size_t globalWorkSize[1] = { static_cast<size_t>(size.y) };
    size_t localWorkSize[1] = { 32 };
	
	// Acquire the GL Object
	// Note, we should ensure GL is completed with any commands that might affect this VBO
	// before we issue OpenCL commands
	auto vboMem = mClVbo->getId();

	errNum = clEnqueueAcquireGLObjects( mClCommandQueue->getId(), 1, &vboMem, 0, NULL, NULL );
	
    // Queue the kernel up for execution across the array
    errNum = clEnqueueNDRangeKernel(mClCommandQueue->getId(), kernel->getId(), 1, NULL,
                                    globalWorkSize, localWorkSize,
                                    0, NULL, NULL);
    if (errNum != CL_SUCCESS)
    {
        std::cerr << "Error queuing kernel for execution." << std::endl;
    }
	
	// Release the GL Object
	// Note, we should ensure OpenCL is finished with any commands that might affect the VBO
	errNum = clEnqueueReleaseGLObjects( mClCommandQueue->getId(), 1, &vboMem, 0, NULL, NULL );
}

void OpenGLInteropApp::mouseDown( MouseEvent event )
{
}

void OpenGLInteropApp::update()
{
//	computeTexture();
	computeVbo();
	
//	auto map = (Vec2f*)mGlVbo->map( GL_READ_ONLY );
//	for( int i = 0; i < mGlVbo->getSize() / 2; ++i ) {
//		std::cout << " Vert: " << *map << endl;
//	}
//	mGlVbo->unmap();
}

void OpenGLInteropApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
//	displayTexture(imWidth,imHeight);
	gl::ScopedVao scopeVao( mGlVao );
	gl::ScopedGlslProg scope( mGlGlsl );
	
	gl::setMatricesWindow( getWindowSize() );
	gl::setDefaultShaderVars();
	
	gl::drawArrays( GL_LINES, 0, getWindowHeight() * 2 );
}

void prepareSettings( App::Settings *settings ) { settings->setWindowSize( 256, 256 ); }

CINDER_APP( OpenGLInteropApp, RendererGl, &prepareSettings )
