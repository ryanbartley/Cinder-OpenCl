#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"

#include "cinder/Rand.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/BufferTexture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/CameraUi.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "Cinder/Utilities.h"
#include "Cinder/Log.h"

#include "Cinder-OpenCL.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma OPENCL EXTENSION cl_khr_gl_event : enable

const int FLOCK_SIZE = 8192;
const int NUM_THREADS = 256;

class BasicFlockingApp : public App {
  public:
	void resize() override;
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void mouseMove( MouseEvent event ) override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	void setupCl();
	static void contextInfo( const char *info, const void *private_info, ::size_t cb, void *user_data) { CI_LOG_E( info ); }
	
  private:
	cl::Platform			mPlatform;
	cl::Context				mContext;
	cl::CommandQueue		mCommandQueue;
	
	CameraUi				mMayaCam;
	CameraPersp				mCam;
	
	cl::Kernel				mUpdateKernel;
	gl::GlslProgRef			mRenderShader;

	gl::VaoRef				mRenderVao;
	
	gl::VboRef				mFlockPosition;
	gl::VboRef				mFlockVelocity;
	
	cl::BufferGL			mFlockPositionBuffer;
	cl::BufferGL			mFlockVelocityBuffer;
	
	gl::BatchRef			mBatch;
			
	std::uint32_t			mCurrIndex = 0;
	std::uint32_t			mDestIndex = 1;
	
	double					mTime;
	
	params::InterfaceGlRef	mParams;
	float					mZoneRadius;
	float					mZoneRadiusSqrd;
	float					mRepelStrength;
	float					mAlignStrength;
	float					mAttractStrength;
	float					mMinThresh;
	float					mMaxThresh;
	float					mDamping;
	float					mTimeMulti;
};

void BasicFlockingApp::resize()
{
	// adjust aspect ratio
	mCam.setAspectRatio( getWindowAspectRatio() );
}

// For generating initial position and velocity data
static void GenerateInitialData( vec4 *posData, vec4 *velData, unsigned int num, float scale )
{
	vec4 *p = posData;
	vec4 *v = velData;
	for( int n = 0; n < num; n++ ){
		*p++ = vec4( Rand::randVec3() * scale, 1.0f );//randFloat( scale * 0.5f, scale );
		*v++ = vec4( Rand::randVec3() * 0.2f, 0.0f );
	}
}

void BasicFlockingApp::setupCl()
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
						   &BasicFlockingApp::contextInfo );
	
	// Create a command-queue on the on the created context and allow for profiling
	mCommandQueue = cl::CommandQueue( mContext, CL_QUEUE_PROFILING_ENABLE );
}

void BasicFlockingApp::setup()
{
	setWindowSize( 1600, 1000 );
	
	setupCl();
	
	// Time
	mTime = getElapsedSeconds();
	
	// Create initial data for buffers
	vec4 *positionData = new vec4[ FLOCK_SIZE ];
	vec4 *velocityData = new vec4[ FLOCK_SIZE ];
	GenerateInitialData( positionData, velocityData, FLOCK_SIZE, 30.0f );
	
	// Create buffers
	mFlockPosition = gl::Vbo::create( GL_ARRAY_BUFFER, FLOCK_SIZE * sizeof( vec4 ), positionData, GL_STATIC_DRAW );
	mFlockVelocity = gl::Vbo::create( GL_ARRAY_BUFFER, FLOCK_SIZE * sizeof( vec4 ), velocityData, GL_STATIC_DRAW );
	
	mFlockPositionBuffer = cl::BufferGL( mContext, CL_MEM_READ_WRITE, mFlockPosition->getId() );
	mFlockVelocityBuffer = cl::BufferGL( mContext, CL_MEM_READ_WRITE, mFlockVelocity->getId() );

	// Setup Render Vaos
	for( int i=0; i<2; i++ )
	{
		mRenderVao = gl::Vao::create();
		gl::ScopedVao vao( mRenderVao );
		{
			gl::ScopedBuffer buffer( mFlockPosition );
			gl::enableVertexAttribArray( 0 );
			gl::vertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, nullptr );
		}
		
		{
			gl::ScopedBuffer buffer( mFlockVelocity );
			gl::enableVertexAttribArray( 1 );
			gl::vertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 0, nullptr );
		}
	}
	// Open up the file
	auto programString = loadString( ci::app::loadAsset( "update.cl" ) );
	
	// Compile OpenCL code
	cl::Program program = cl::Program( mContext, programString );
	
	program.build("-cl-fast-relaxed-math");
	
	mUpdateKernel = cl::Kernel( program, "smartUpdate" );
	mUpdateKernel.setArg( 0, mFlockPositionBuffer );
	mUpdateKernel.setArg( 1, mFlockVelocityBuffer );
	mUpdateKernel.setArg( 2, sizeof(int), &FLOCK_SIZE );
	mUpdateKernel.setArg( 11, sizeof(vec4)*NUM_THREADS, nullptr );
	mUpdateKernel.setArg( 12, sizeof(vec4)*NUM_THREADS, nullptr );
	
	mRenderShader = gl::GlslProg::create( gl::GlslProg::Format()
										 .vertex( loadAsset( "render.vert" ) )
										 .fragment( loadAsset( "render.frag" ) ) );
//										 .attribLocation( "vInstancePosition", 0 )
//										 .attribLocation( "vInstanceVelocity", 1 ) );
//	THIS STUFF IS COMMENTED OUT BECAUSE I DONT KNOW HOW TO DO INSTANCING PROPERLY.
//	SO FOR NOW IM JUST DRAWING GL_POINTS
//
//	gl::VboMeshRef cubeMesh	= gl::VboMesh::create( geom::Cube().size( vec3( 0.2f, 0.2f, 0.2f ) ) );
//	geom::BufferLayout instanceDataLayout;
//	instanceDataLayout.append( geom::Attrib::CUSTOM_0, 3, 0, 0, 1 );
//	instanceDataLayout.append( geom::Attrib::CUSTOM_1, 3, 0, 0, 1 );
//	cubeMesh->appendVbo( instanceDataLayout, mFlockPosition[0] );
//	
//	mBatch = gl::Batch::create( cubeMesh, mRenderShader, { { geom::Attrib::CUSTOM_0, "vInstancePosition" }, { geom::Attrib::CUSTOM_1, "vInstanceVelocity" } } );
	
	delete [] positionData;
	delete [] velocityData;
	
	mZoneRadius			= 2.0f;
	mZoneRadiusSqrd		= mZoneRadius * mZoneRadius;
	mMinThresh			= 0.45f;
	mMaxThresh			= 0.80f;
	mRepelStrength		= 0.11f;
	mAlignStrength		= 0.30f;
	mAttractStrength	= 0.35f;
	mDamping			= 0.99f;
	mTimeMulti			= 6.0f;
	
	mParams = params::InterfaceGl::create( getWindow(), "Flocking Params", ivec2( 200, 300 ) );
	mParams->addParam( "Zone Radius",		&mZoneRadius ).min( 2.0f ).max( 10.0f ).precision( 2 ).step( 0.5f );
	mParams->addParam( "Min Thresh",		&mMinThresh ).min( 0.1f ).max( 0.8f ).precision( 2 ).step( 0.01f );
	mParams->addParam( "Max Thresh",		&mMaxThresh ).min( 0.2f ).max( 1.0f ).precision( 2 ).step( 0.01f );
	mParams->addParam( "Repel Strength",	&mRepelStrength ).min( 0.01f ).max( 1.0f ).precision( 2 ).step( 0.01f );
	mParams->addParam( "Align Strength",	&mAlignStrength ).min( 0.01f ).max( 1.0f ).precision( 2 ).step( 0.01f );
	mParams->addParam( "Attract Strength",	&mAttractStrength ).min( 0.01f ).max( 1.0f ).precision( 2 ).step( 0.01f );
	mParams->addParam( "Damping",			&mDamping ).min( 0.9f ).max( 0.999f ).precision( 4 ).step( 0.001f );
	mParams->addParam( "Time Multi",		&mTimeMulti ).min( 1.0f ).max( 30.0f ).precision( 2 ).step( 1.0f );
	
	// CameraPersp
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mCam.lookAt( vec3( 0.0f, 0.0f, -70.0f ), vec3( 0.0f ) );
	mMayaCam.setCamera( &mCam );
}

void BasicFlockingApp::mouseMove( MouseEvent event )
{
}

void BasicFlockingApp::mouseDown( MouseEvent event )
{	
	// let the camera handle the interaction
	mMayaCam.mouseDown( event.getPos() );
}

void BasicFlockingApp::mouseDrag( MouseEvent event )
{	
	mouseMove( event );
	// let the camera handle the interaction
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void BasicFlockingApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f' ){
		setFullScreen( ! isFullScreen() );
	}
}

void BasicFlockingApp::update()
{
	double prevTime = mTime;
	mTime			= getElapsedSeconds();
	float dt		= ( mTime - prevTime ) * mTimeMulti;
//	float dt		= ( 1.0f/10.0f ) * mTimeMulti;
	
	mZoneRadiusSqrd = mZoneRadius * mZoneRadius;
	if( mMinThresh >= mMaxThresh ) mMinThresh = mMaxThresh - 0.01f;
	
	std::vector<cl::Memory> glObjects = { mFlockPositionBuffer, mFlockVelocityBuffer };
	try {
		// Update particles on the GPU
		mCommandQueue.enqueueAcquireGLObjects( &glObjects );
		
		// Bind the source data (Attributes refer to specific buffers).
		mUpdateKernel.setArg( 3, sizeof(float), &mDamping );
		mUpdateKernel.setArg( 4, sizeof(float), &mZoneRadiusSqrd );
		mUpdateKernel.setArg( 5, sizeof(float), &mRepelStrength );
		mUpdateKernel.setArg( 6, sizeof(float), &mAlignStrength );
		mUpdateKernel.setArg( 7, sizeof(float), &mAttractStrength );
		mUpdateKernel.setArg( 8, sizeof(float), &mMinThresh );
		mUpdateKernel.setArg( 9, sizeof(float), &mMaxThresh );
		mUpdateKernel.setArg( 10, sizeof(float), &dt );
		
		mCommandQueue.enqueueNDRangeKernel( mUpdateKernel, cl::NullRange, cl::NDRange( FLOCK_SIZE ), cl::NDRange( NUM_THREADS ) );
		
		mCommandQueue.enqueueReleaseGLObjects( &glObjects );
	}
	catch( const cl::Error &e ) {
		CI_LOG_E( e.what() << " and error " << errorToString( e.err() ) );
	}
	
}

void BasicFlockingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::ScopedMatrices push;
	gl::setMatrices( mMayaCam.getCamera() );
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	gl::ScopedGlslProg render( mRenderShader );
	gl::ScopedVao vao( mRenderVao );
	gl::setDefaultShaderVars();
	gl::drawArrays( GL_POINTS, 0, FLOCK_SIZE );
//	mBatch->drawInstanced( FLOCK_SIZE );
	
//	mParams->draw();
	
	if( getElapsedFrames() % 60 == 59 ) std::cout << "FPS: " << getAverageFps() << std::endl;
}

CINDER_APP( BasicFlockingApp, RendererGl, []( BasicFlockingApp::Settings *settings ){
	settings->setWindowSize( 1600, 900 );
} )
