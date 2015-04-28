#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CommandQueue.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/GlslProg.h"
#include "BufferObj.h"
#include "cinder/gl/Vbo.h"
#include "Program.h"

#include "cinder/gl/Batch.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const float MAX_DELTA_TIME = .1f;
const int PARTICLE_COUNT = 512*512;
const float BASE_LIFETIME = 2.f;
const float MAX_LIFETIME = 4.;
const int NUM_SLICES = 128;

class BitonicParticleSortingApp : public App {
public:
    BitonicParticleSortingApp();
    void mouseDown( MouseEvent event ) override;
    void update() override;
    void draw() override;
    
    CameraPersp			mCam;
    
    cl::PlatformRef		mClPlatform;
    cl::ContextRef		mClContext;
    cl::CommandQueueRef mClCommandQueue;
    cl::ProgramRef      mClProgram;
    
    cl::BufferObjRef    mClPositions, mClStartPositions;
    gl::VboRef          mPositions, mStartPositions;
    
    gl::BatchRef        mParticles;
    gl::GlslProgRef     mRender;
    
    float               mDeltaTime;
    float               mPrevTime;
    
    cl_mem				mGlObject;
    
};

BitonicParticleSortingApp::BitonicParticleSortingApp()
{
    
    //OpenGL
    
    vector<vec4> data;
    for( int i = 0; i < PARTICLE_COUNT; i++ ){
        vec3 rand = randVec3f();
        data.push_back( vec4( rand*2.f, BASE_LIFETIME + BASE_LIFETIME*randFloat() ) );
    }
    
    mPositions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4)*data.size(), data.data(), GL_DYNAMIC_COPY );
    mStartPositions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4)*data.size(), data.data(), GL_STATIC_DRAW );
    
    mRender = gl::GlslProg::create( loadAsset("render.vert"), loadAsset("render.frag") );
    
    geom::BufferLayout pos_layout;
    pos_layout.append(geom::POSITION, 4, 0, 0);
    vector<pair<geom::BufferLayout, gl::VboRef>> buffers(1, make_pair(pos_layout, mPositions) );
    
    auto mesh = gl::VboMesh::create( PARTICLE_COUNT, GL_POINTS, buffers );
    
    mParticles = gl::Batch::create(mesh, mRender);
    
    mRender->uniform("uParticleDiameter", .1f);
    mRender->uniform("uParticleSizeFactor", 3.f);
    mRender->uniform("uMaxLife", MAX_LIFETIME );
    
    mCam.setPerspective(60, getWindowAspectRatio(), .1, 10000);
    mCam.lookAt(vec3(0,0,5), vec3(0));
    
    //OpenCL
    
    mClPlatform = cl::Platform::create( cl::Platform::getAvailablePlatforms()[0], true );
    mClContext = cl::Context::create( mClPlatform, true );
    mClCommandQueue = cl::CommandQueue::create( mClContext->getAssociatedDevices()[0] );
    
    mClPositions = cl::BufferObj::create( mPositions, CL_MEM_READ_WRITE );
    mClStartPositions = cl::BufferObj::create( mStartPositions, CL_MEM_READ_ONLY );
    
    mClProgram = cl::Program::create( loadAsset( "particleSort.cl" ) );
    auto kernel = mClProgram->createKernel( "particle_update" );
    
    mGlObject = mClPositions->getId();
    
    kernel->setKernelArg( 0, mClPositions );
    kernel->setKernelArg( 1, mClStartPositions );
    
    auto sort = mClProgram->createKernel( "bitonic_sort" );
    sort->setKernelArg( 0, mClPositions );
    
    mDeltaTime = 0.;
    mPrevTime = 0.;
    
    gl::enableAlphaBlending();
    gl::enable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

void BitonicParticleSortingApp::mouseDown( MouseEvent event )
{
}

void BitonicParticleSortingApp::update()
{
    auto simulation_kernel = mClProgram->getKernelByName( "particle_update" );
    
    auto time = getElapsedSeconds();
    
    mDeltaTime = getElapsedSeconds() - mPrevTime;
    mPrevTime = getElapsedSeconds();
    
    if (mDeltaTime > MAX_DELTA_TIME) {
        mDeltaTime = 0;
    }
    
    simulation_kernel->setKernelArg( 2, sizeof(float), &time );
    simulation_kernel->setKernelArg( 3, sizeof(float), &mDeltaTime );
    
    size_t globalWorkSize[1] = { static_cast<size_t>(PARTICLE_COUNT) };
    std::vector<cl::MemoryObjRef> vboMem(1, mClPositions);
    
    glFinish(); //why?
    
    cl::Event event;
    mClCommandQueue->acquireGlObjects( vboMem, {}, &event );
    
    cl::EventList waitList({ event });
    cl::Event simulationKernelEvent;
    mClCommandQueue->NDRangeKernel( simulation_kernel, 1, nullptr, globalWorkSize, nullptr , waitList, &simulationKernelEvent );
    
    waitList.getList().push_back( simulationKernelEvent );
    
    ///Perform sort
    
    cl_uint sortOrder = 0; // descending order else 1 for ascending order
    cl_uint stages = 0;
    for(unsigned int i = PARTICLE_COUNT; i > 1; i >>= 1)
        ++stages;
    
    auto sort_kernel = mClProgram->getKernelByName("bitonic_sort");
    
    sort_kernel->setKernelArg( 3, sizeof(cl_uint), (void*)&sortOrder );
    
    for(cl_uint stage = 0; stage < stages; ++stage) {
        
        sort_kernel->setKernelArg(1,  sizeof(cl_uint),(void*)&stage);
        
        for(cl_uint subStage = 0; subStage < stage +1; subStage++) {
            
            sort_kernel->setKernelArg(2,  sizeof(cl_uint),(void*)&subStage);
            
            cl::Event sortPass;
            
            mClCommandQueue->NDRangeKernel( sort_kernel, 1, nullptr, globalWorkSize, nullptr, waitList, &sortPass );
            //            mClCommandQueue->waitForEvents( cl::EventList({ simulationKernelEvent, sortPass }) );
            waitList.getList().push_back( sortPass );
            
        }
    }
    
    mClCommandQueue->finish();
    mClCommandQueue->releaseGlObjects( vboMem, waitList );
    
}

void BitonicParticleSortingApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    gl::ScopedMatrices pushMatrices;
    gl::setMatrices(mCam);
    gl::ScopedViewport v( vec2(0), getWindowSize() );
    
    gl::ScopedModelMatrix model;
    gl::multModelMatrix(rotate( (float)getElapsedSeconds(), vec3(1)));
    
    for(int i = 0; i < NUM_SLICES; i++ ){
        
        auto glsl = mParticles->getGlslProg();
        // glsl->uniform("uColor", vec3(1));
        glsl->uniform("uColor", vec3( (float)i / (float)NUM_SLICES, 1.f - (float)i / (float)NUM_SLICES, 1.f ) );
        mParticles->draw( i * (PARTICLE_COUNT / NUM_SLICES), PARTICLE_COUNT / NUM_SLICES );
        
    }
    
    if(getElapsedFrames()%30==0)console() << getAverageFps() << endl;
    
}

void prepareSettings( App::Settings* settings )
{
    
}

CINDER_APP( BitonicParticleSortingApp, RendererGl )
