#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/CameraUi.h"
#include "cinder/Log.h"
#include "cinder/gl/ConstantConversions.h"
#include "cinder/params/Params.h"

#include "Cinder-OpenCL.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// This sample comes from an apple sample, which you can find here...
// https://developer.apple.com/library/mac/samplecode/OpenCL_Procedural_Geometric_Displacement_Example/Listings/ReadMe_txt.html
// and mixes it with the CubeMapping sample from Cinder. Setup of cl
// is similar to each other sample. 

class ProceduralGeometricDisplacementApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
	
	void setupCl();
	void setupBuffers();
	void setupKernels();
	void setupParams();
	
	static void contextErrorCallback( const char *errinfo, const void *private_info,
									 ::size_t cb, void *user_data )
	{ CI_LOG_E( errinfo ); }
	
	CameraPersp		mCam;
	CameraUi		mCameraControl;
	
	gl::BatchRef			mSphereEnv, mSkyBatch;
	gl::TextureCubeMapRef	mSkyBox;
	
	params::InterfaceGlRef	mParams;
	
	cl::Context			mContext;
	cl::Kernel			mComputeKernel;
	cl::Program			mComputeProgram;
	cl::CommandQueue	mQueue;
	cl::Buffer			mInputVertexBuffer;
	cl::BufferGL		mOutputVertexBuffer,
						mOutputNormalBuffer;
	
	size_t		mSphereNumVertices;
	
	float	mFrequency	= 1.0f;
	float	mAmplitude  = 0.35f;
	float	mOctaves    = 5.5f;
	float	mRoughness  = 0.025f;
	float	mLacunarity = 2.0f;
	float	mIncrement  = 1.5f;
	float	mPhase      = 0.0f;
	float	mPhaseIncrease = 0.001f;
};

void ProceduralGeometricDisplacementApp::setup()
{
	setupCl();
	setupBuffers();
	setupKernels();
	setupParams();
	
	mCam.lookAt( vec3( 0.0f, 5.0f, -7.0f ), vec3( 0 ) );
	mCam.setPerspective( 55.0f, getWindowAspectRatio(), 0.1f, 10000.0f );
	
	mSkyBox = gl::TextureCubeMap::create( loadImage( loadAsset( "env_map.jpg" ) ),
										 gl::TextureCubeMap::Format().mipmap() );
	
	mCameraControl.setCamera( &mCam );
	mCameraControl.connect( getWindow() );
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void ProceduralGeometricDisplacementApp::setupCl()
{
	// This setup is the same as in HelloWorld with the shared context of OpenGlInterop.
	std::vector<cl::Platform> platforms;
	cl::Platform::get( &platforms );
	
	std::vector<cl::Device> devices;
	platforms[0].getDevices( CL_DEVICE_TYPE_GPU, &devices );
	
	mContext = cl::Context( devices[0],
						   ocl::getDefaultSharedGraphicsContextProperties( platforms[0] ),
						   &ProceduralGeometricDisplacementApp::contextErrorCallback );

	mQueue = cl::CommandQueue( mContext );
}

void ProceduralGeometricDisplacementApp::setupBuffers()
{
	auto environmentMap = gl::GlslProg::create( loadAsset( "env_map.vert" ), loadAsset( "env_map.frag" ) );
	environmentMap->uniform( "uCubeMapTex", 0 );
	auto skyBoxGlsl		= gl::GlslProg::create( loadAsset( "sky_box.vert" ), loadAsset( "sky_box.frag" ) );
	skyBoxGlsl->uniform( "uCubeMapTex", 0 );
	
	auto sky = geom::Cube().size( vec3( 500 ) );
	mSkyBatch = gl::Batch::create( sky, skyBoxGlsl );
	// Create the sphere
	auto sphere = geom::Sphere().subdivisions( 500 );
	auto sphereIndexTrimesh = TriMesh::create( sphere, TriMesh::Format().positions(4) );
	
	mSphereNumVertices = sphereIndexTrimesh->getNumVertices();
	
	size_t bytes = mSphereNumVertices * sizeof(vec4);

	// Create the buffers for the position and normals
	auto positionVbo	= gl::Vbo::create( GL_ARRAY_BUFFER, bytes, nullptr, GL_STATIC_DRAW );
	auto posAttrib		= geom::AttribInfo( geom::POSITION, geom::FLOAT, 4, 0, 0 );
	auto positionLayout = geom::BufferLayout( { posAttrib } );
	auto normalVbo		= gl::Vbo::create( GL_ARRAY_BUFFER, bytes, nullptr, GL_STATIC_DRAW );
	auto normalAttrib	= geom::AttribInfo( geom::NORMAL, geom::FLOAT, 4, 0, 0 );
	auto normalLayout	= geom::BufferLayout( { normalAttrib } );
	auto indicesVbo		= gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER,
										  sphereIndexTrimesh->getNumIndices() * sizeof(uint32_t),
										  sphereIndexTrimesh->getIndices().data(), GL_STATIC_DRAW );
	
	auto vboMesh		= gl::VboMesh::create( mSphereNumVertices, GL_TRIANGLES,
											  { { positionLayout, positionVbo }, { normalLayout, normalVbo } },
											  sphereIndexTrimesh->getNumIndices(), GL_UNSIGNED_INT, indicesVbo );

	mSphereEnv		= gl::Batch::create( vboMesh, environmentMap );
	
	mInputVertexBuffer = cl::Buffer( mContext, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
									bytes, sphereIndexTrimesh->getBufferPositions().data() );
	mOutputVertexBuffer = cl::BufferGL( mContext, CL_MEM_READ_WRITE, positionVbo->getId() );
	mOutputNormalBuffer = cl::BufferGL( mContext, CL_MEM_READ_WRITE, normalVbo->getId() );
}

void ProceduralGeometricDisplacementApp::setupKernels()
{
	// Create OpenCL program from HelloWorld.cl kernel source
	mComputeProgram = cl::Program( mContext, loadString( loadAsset( "displacement_kernel.cl" ) ), true );
	
	mComputeKernel = cl::Kernel( mComputeProgram, "displace" );
	mComputeKernel.setArg( 0, mInputVertexBuffer );
	mComputeKernel.setArg( 1, mOutputNormalBuffer );
	mComputeKernel.setArg( 2, mOutputVertexBuffer );
}

void ProceduralGeometricDisplacementApp::update()
{
	std::vector<cl::Memory> aqcuire = { mOutputNormalBuffer, mOutputVertexBuffer };
	mQueue.enqueueAcquireGLObjects( &aqcuire );
	
	mComputeKernel.setArg( 3, sizeof(float), &mFrequency );
	mComputeKernel.setArg( 4, sizeof(float), &mAmplitude );
	mComputeKernel.setArg( 5, sizeof(float), &mPhase );
	mComputeKernel.setArg( 6, sizeof(float), &mLacunarity );
	mComputeKernel.setArg( 7, sizeof(float), &mIncrement );
	mComputeKernel.setArg( 8, sizeof(float), &mOctaves );
	mComputeKernel.setArg( 9, sizeof(float), &mRoughness );
	
	mQueue.enqueueNDRangeKernel( mComputeKernel,
								cl::NullRange,
								cl::NDRange( mSphereNumVertices ) );
	
	mQueue.enqueueReleaseGLObjects( &aqcuire );
	mPhase += mPhaseIncrease;
}

void ProceduralGeometricDisplacementApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::ScopedMatrices scopeMat;
	gl::setMatrices( mCam );
	
	gl::ScopedTextureBind scopeTex( mSkyBox );
	mSkyBatch->draw();
	{
		gl::ScopedModelMatrix scopeModel;
		gl::scale( vec3( 2.0 ) );
		mSphereEnv->draw();
	}
	
	mParams->draw();
}

void ProceduralGeometricDisplacementApp::setupParams()
{
	static float sResetFrequency = mFrequency;
	static float sResetAmplitude = mAmplitude;
	static float sResetOctaves = mOctaves;
	static float sResetRoughness = mRoughness;
	static float sResetLacunarity = mLacunarity;
	static float sResetIncrement = mIncrement;
	static float sResetPhaseIncrease = mPhaseIncrease;
	mParams = params::InterfaceGl::create( "Compute", vec2( 200, 200 ) );
	mParams->addParam( "Frequency", &mFrequency ).min( 0.0f ).step( 0.01f );
	mParams->addParam( "Amplitude", &mAmplitude ).min( 0.0f ).max( 7.0f ).step( 0.01f );
	mParams->addParam( "Octaves", &mOctaves ).min( 0.0f ).max( 7.0f ).step( 0.01f );
	mParams->addParam( "Roughness", &mRoughness );
	mParams->addParam( "Lacunarity", &mLacunarity );
	mParams->addParam( "Increment", &mIncrement ).min( 0.01f ).step( 0.05f );
	mParams->addParam( "Phase Increase", &mPhaseIncrease ).min( 0.0f ).step( 0.01f );
	mParams->addButton( "Reset", [&]() {
		mFrequency = sResetFrequency;
		mAmplitude = sResetAmplitude;
		mOctaves = sResetOctaves;
		mRoughness = sResetRoughness;
		mLacunarity = sResetLacunarity;
		mIncrement = sResetIncrement;
		mPhaseIncrease = sResetPhaseIncrease;
	});
}

CINDER_APP( ProceduralGeometricDisplacementApp, RendererGl,
[]( App::Settings *settings ) {
	settings->setWindowSize( ivec2( 1024 ) );
} )
