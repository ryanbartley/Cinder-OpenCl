#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Rand.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "Cinder/Log.h"

#include "Cinder-OpenCL.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const int FLOCK_SIZE	= 2560;
const int NUM_THREADS	= 256;

class SimpleFlockingApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
	
	void setupCl();
	void setupBuffers();
	void setupClProgram();
	void setupParams();
	
	static void contextInfo( const char *info, const void *private_info, ::size_t cb, void *user_data )
	{ CI_LOG_E( info ); }
	
  private:
	ocl::Context			mContext;
	ocl::CommandQueue		mCommandQueue;
	ocl::Kernel				mUpdateKernel;
	ocl::BufferGL			mClFlockParticle;
	
	gl::BatchRef			mBatch;
	gl::VboRef				mGlFlockParticle;
	
	CameraUi				mMayaCam;
	CameraPersp				mCam;
	
	params::InterfaceGlRef	mParams;
	float mZoneRadius			= 3.0f;
	float mMinThresh			= 0.28f;
	float mMaxThresh			= 0.46f;
	float mRepelStrength		= 0.5f;
	float mAlignStrength		= 0.62f;
	float mAttractStrength		= 0.13f;
	float mDamping				= 0.99f;
	float mTimeMulti			= 6.0f;
};

void SimpleFlockingApp::setupCl()
{
	// Get all of the platforms on this system
	std::vector<ocl::Platform> platforms;
	ocl::Platform::get( &platforms );
	
	std::vector<ocl::Device> devices;
	platforms[0].getDevices( CL_DEVICE_TYPE_GPU, &devices );
	
	mContext = ocl::Context( devices[0],
							ocl::getDefaultSharedGraphicsContextProperties( platforms[0] ),
							&SimpleFlockingApp::contextInfo );
	
	CI_LOG_I( "Max Compute Units: " << devices[0].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() );
	CI_LOG_I( "Max Local Memory: " << devices[0].getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() );
	CI_LOG_I( "Max Work Group Size: " << devices[0].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() );
	
	mCommandQueue = ocl::CommandQueue( mContext, CL_QUEUE_PROFILING_ENABLE );
}

void SimpleFlockingApp::setupBuffers()
{
	//
	struct Particle {
		cl_float4 pos;
		cl_float4 vel;
	};
	
	// Create initial data for buffers
	std::array<Particle, FLOCK_SIZE> particleData;
	auto ptr = particleData.data();
	for( int n = 0; n < FLOCK_SIZE; n++, ptr++ ){
		ptr->pos = ocl::toCl( vec4( Rand::randVec3() * 30.0f, 1.0f ) );
		ptr->vel = ocl::toCl( vec4( Rand::randVec3() * 0.2f, 0.0f ) );
	}
	
	// Create buffers
	mGlFlockParticle = gl::Vbo::create( GL_ARRAY_BUFFER, FLOCK_SIZE * sizeof( Particle ), particleData.data(), GL_STATIC_DRAW );
	mClFlockParticle = ocl::BufferGL( mContext, CL_MEM_READ_WRITE, mGlFlockParticle->getId() );
	
	auto cone = geom::Cone().set( vec3( 0, -.5, 0 ), vec3( 0, .5, 0 ) );
	auto coneGeom = cone >> geom::Scale( vec3( .25, .5, .25 ) );
	auto coneMesh = gl::VboMesh::create( coneGeom );
	
	geom::BufferLayout instanceDataLayout;
	instanceDataLayout.append( geom::Attrib::CUSTOM_0, 4, sizeof( Particle ), offsetof(Particle, pos), 1 );
	instanceDataLayout.append( geom::Attrib::CUSTOM_1, 4, sizeof( Particle ), offsetof(Particle, vel), 1 );
	coneMesh->appendVbo( instanceDataLayout, mGlFlockParticle );
	
	auto shader = gl::GlslProg::create( gl::GlslProg::Format()
										.vertex( loadAsset( "render.vert" ) )
										.fragment( loadAsset( "render.frag" ) ) );
	
	mBatch = gl::Batch::create( coneMesh, shader,
							   { { geom::Attrib::CUSTOM_0, "instPosition" },
								   { geom::Attrib::CUSTOM_1, "instVelocity" } } );
}

void SimpleFlockingApp::setupClProgram()
{
	// Compile OpenCL code
	auto program = ocl::createProgram( mContext, loadAsset( "update.cl" ), true );
	
	program.build("-cl-fast-relaxed-math -cl-kernel-arg-info");
	
	mUpdateKernel = ocl::Kernel( program, "update" );
	// output info about the kernel
	cout << mUpdateKernel << endl;
	mUpdateKernel.setArg( 0, mClFlockParticle );
	mUpdateKernel.setArg( 1, sizeof(int), &FLOCK_SIZE );
	//	mUpdateKernel.setArg( 11, sizeof(vec4)*NUM_THREADS, nullptr );
}

void SimpleFlockingApp::setup()
{
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	setupCl();
	setupBuffers();
	setupClProgram();
	setupParams();
	
	// CameraPersp
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mCam.lookAt( vec3( 0.0f, 0.0f, -70.0f ), vec3( 0.0f ) );
	
	mMayaCam.setCamera( &mCam );
	mMayaCam.connect( getWindow() );
}

void SimpleFlockingApp::update()
{
	float dt = (1.0 / 60.0) * mTimeMulti;
	
	std::vector<ocl::Memory> glObjects = { mClFlockParticle };
	try {
		float zoneRadiusSqd = mZoneRadius * mZoneRadius;
		// Update particles on the GPU
		mCommandQueue.enqueueAcquireGLObjects( &glObjects );
		// Bind the source data (Attributes refer to specific buffers).
		mUpdateKernel.setArg( 2, sizeof(float), &mDamping );
		mUpdateKernel.setArg( 3, sizeof(float), &zoneRadiusSqd );
		mUpdateKernel.setArg( 4, sizeof(float), &mRepelStrength );
		mUpdateKernel.setArg( 5, sizeof(float), &mAlignStrength );
		mUpdateKernel.setArg( 6, sizeof(float), &mAttractStrength );
		mUpdateKernel.setArg( 7, sizeof(float), &mMinThresh );
		mUpdateKernel.setArg( 8, sizeof(float), &mMaxThresh );
		mUpdateKernel.setArg( 9, sizeof(float), &dt );
		
		mCommandQueue.enqueueNDRangeKernel( mUpdateKernel,
										   ocl::NullRange,
										   ocl::NDRange( FLOCK_SIZE ),
										   ocl::NDRange( NUM_THREADS ) );
		
		mCommandQueue.enqueueReleaseGLObjects( &glObjects );
	}
	catch( const ocl::Error &e ) {
		CI_LOG_E( e.what() << " and error " << ocl::errorToString( e.err() ) );
	}
}

void SimpleFlockingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::ScopedMatrices push;
	gl::setMatrices( mCam );
	mBatch->drawInstanced( FLOCK_SIZE );
	
	mParams->draw();

}

void SimpleFlockingApp::setupParams()
{
	mParams = params::InterfaceGl::create( getWindow(), "Flocking Params", ivec2( 200, 300 ) );
	mParams->addParam( "Zone Radius",		&mZoneRadius ).min( 2.0f ).max( 10.0f ).precision( 2 ).step( 0.5f );
	mParams->addParam( "Min Thresh",		&mMinThresh ).min( 0.1f ).max( 0.8f ).precision( 2 ).step( 0.01f )
	.updateFn( [&](){
		if( mMinThresh >= mMaxThresh )
			mMinThresh = mMaxThresh - 0.01f;
	});
	mParams->addParam( "Max Thresh",		&mMaxThresh ).min( 0.2f ).max( 1.0f ).precision( 2 ).step( 0.01f )
	.updateFn( [&](){
		if( mMinThresh >= mMaxThresh )
			mMinThresh = mMaxThresh - 0.01f;
	});
	mParams->addParam( "Repel Strength",	&mRepelStrength ).min( 0.01f ).max( 1.0f ).precision( 2 ).step( 0.01f );
	mParams->addParam( "Align Strength",	&mAlignStrength ).min( 0.01f ).max( 1.0f ).precision( 2 ).step( 0.01f );
	mParams->addParam( "Attract Strength",	&mAttractStrength ).min( 0.01f ).max( 1.0f ).precision( 2 ).step( 0.01f );
	mParams->addParam( "Damping",			&mDamping ).min( 0.9f ).max( 0.999f ).precision( 4 ).step( 0.001f );
	mParams->addParam( "Time Multi",		&mTimeMulti ).min( 1.0f ).max( 30.0f ).precision( 2 ).step( 1.0f );
}

CINDER_APP( SimpleFlockingApp, RendererGl, []( SimpleFlockingApp::Settings *settings ){
	settings->setWindowSize( ci::ivec2(1024) );
} )
