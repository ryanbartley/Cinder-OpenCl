//
//  MarchingCubes.cpp
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#include "MarchingCubes.h"
#include "CommandQueue.h"
#include "cinder/Log.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace ci::gl;
using namespace ci::cl;

size_t full_size = width * height * depth;

MarchingCubes::MarchingCubes( const ci::cl::CommandQueueRef &commandQueue )
: mCommandQueue( commandQueue )
{
	std::vector<vec4> pointData(full_size);
	

	int x, y, z;
	for(x = 0; x < width; x++)
	for(y = 0; y < height; y++)
	for(z = 0; z < depth; z++) {
		int id = x + y * width + z * width * height;
		vec4 position = vec4(x, y, z, 1);
		pointData[id] = position;
	}

	mGlPointPositions = gl::Vbo::create(GL_ARRAY_BUFFER,
										sizeof(vec4) * full_size,
										pointData.data(),
										GL_STATIC_DRAW);
	
	std::vector<vec4> pointColorData(full_size);
	memset( pointColorData.data(), 0, sizeof(vec4) * full_size );
	mGlPointColors = gl::Vbo::create(GL_ARRAY_BUFFER,
									 sizeof(vec4) * full_size,
									 pointColorData.data(),
									 GL_DYNAMIC_COPY);
	
	mClPointColors = cl::BufferObj::create( mGlPointColors, CL_MEM_READ_WRITE );
	
	mClVolume = cl::BufferObj::create( CL_MEM_READ_WRITE, sizeof(float) * full_size, nullptr );
	
	std::vector<vec4> vertPosData(sizeof(vec4) * MAX_VERTS);
	memset(vertPosData.data(), 0, sizeof(vec4) * MAX_VERTS);
	mGlVertPositions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * MAX_VERTS, vertPosData.data(), GL_DYNAMIC_COPY);

	mClVertPositions = cl::BufferObj::create( mGlVertPositions, CL_MEM_READ_WRITE );
	
	std::vector<vec4> vertNormData(sizeof(vec4) * MAX_VERTS);
	memset(vertNormData.data(), 0, sizeof(vec4) * MAX_VERTS);
	mGlVertNormals = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * MAX_VERTS, vertNormData.data(), GL_DYNAMIC_COPY);
	
	mClVertNormals = cl::BufferObj::create( mGlVertNormals, CL_MEM_READ_WRITE );
	
	mClVertIndex = cl::BufferObj::create( CL_MEM_READ_WRITE, sizeof(int), nullptr );
	
	mClProgram = cl::Program::create( loadAsset( "marching_cubes.cl" ) );
	
	mKernWritePoint = mClProgram->createKernel( "write_point" );
	mKernWritePoint->setKernelArg( 0, mClVolume );
	mKernWritePoint->setKernelArg( 4, sizeof(int), (void*)&width );
	mKernWritePoint->setKernelArg( 5, sizeof(int), (void*)&height );
	mKernWritePoint->setKernelArg( 6, sizeof(int), (void*)&depth );
	
	mKernWriteMetaball = mClProgram->createKernel( "write_metaball" );
	mKernWriteMetaball->setKernelArg( 0, mClVolume );
	
	mKernWriteMetaballs = mClProgram->createKernel( "write_metaballs" );
	mKernWriteMetaballs->setKernelArg( 0, mClVolume );
	
	mKernWriteClear = mClProgram->createKernel( "write_clear" );
	mKernWriteClear->setKernelArg( 0, mClVolume );
	
	mKernWritePointColorBack = mClProgram->createKernel( "write_point_color_back" );
	mKernWritePointColorBack->setKernelArg( 0, mClVolume );
	mKernWritePointColorBack->setKernelArg( 1, mClPointColors );

	mKernConstructSurface = mClProgram->createKernel( "construct_surface" );
	mKernConstructSurface->setKernelArg( 0, mClVolume );
	
	mKernGenNormals = mClProgram->createKernel( "generate_flat_normals" );
	
	mKernGenNormalsSmooth = mClProgram->createKernel( "generate_smoot_normals" );
}

void MarchingCubes::clear()
{
	mCommandQueue->NDRangeKernel( mKernWriteClear, 1, nullptr, &full_size, nullptr );
}

void MarchingCubes::point( const Vec4i &point ) {

	if ((point.x >= width) || (point.y >= height) || (point.z >= depth) || (point.x < 0) || (point.y < 0) || (point.z < 0)) {
		CI_LOG_E("Point outside of volume");
	}

	mKernWritePoint->setKernelArg( 1, sizeof(int), (void*)&point.x );
	mKernWritePoint->setKernelArg( 2, sizeof(int), (void*)&point.y );
	mKernWritePoint->setKernelArg( 3, sizeof(int), (void*)&point.z );
	mKernWritePoint->setKernelArg( 7, sizeof(float), (void*)&point.w );
	static size_t work_size = 1;
	mCommandQueue->NDRangeKernel( mKernWritePoint, 1, nullptr, &work_size, nullptr );
}

//void MarchingCubes::metaball(float x, float y, float z) {
//	const int METABALL_SIZE = 10;
//
//	int bot_x = math<int>::max(floor(x) - METABALL_SIZE, 0);
//	int bot_y = math<int>::max(floor(y) - METABALL_SIZE, 0);
//	int bot_z = math<int>::max(floor(z) - METABALL_SIZE, 0);
//
//	int top_x = math<int>::min(ceil(x) + METABALL_SIZE, width-1);
//	int top_y = math<int>::min(ceil(y) + METABALL_SIZE, height-1);
//	int top_z = math<int>::min(ceil(z) + METABALL_SIZE, depth-1);
//
//	size_t count = (top_x - bot_x) * (top_y - bot_y) * (top_z - bot_z);
//
//	Vec3i bottom{bot_x, bot_y, bot_z};
//	Vec3i top{top_x, top_y, top_z};
//	Vec3i size{width, height, depth};
//
//	mKernWriteMetaball->setKernelArg( 1, sizeof(Vec3i), &bottom );
//	mKernWriteMetaball->setKernelArg( 2, sizeof(Vec3i), &top);
//	mKernWriteMetaball->setKernelArg( 3, sizeof(Vec3i), &size);
//	mKernWriteMetaball->setKernelArg( 4, sizeof(float), &x);
//	mKernWriteMetaball->setKernelArg( 5, sizeof(float), &y);
//	mKernWriteMetaball->setKernelArg( 6, sizeof(float), &z);
//	mCommandQueue->NDRangeKernel( mKernWriteMetaball, 1, nullptr, &count, nullptr );
//}

void MarchingCubes::marchingCubesMetaballData( const cl::BufferObjRef &positions, int numBalls )
{
	mMetaballPositions = positions;
	mNumBalls = numBalls;
}

void MarchingCubes::update()
{
	Vec3i size{width, height, depth};
	
	/* Update volumes */
	
	vector<cl::MemoryObjRef> acquire{
		mMetaballPositions,
		mClVertPositions,
		mClVertNormals
	};
	
	mCommandQueue->acquireGlObjects( acquire );
	
	mKernWriteMetaballs->setKernelArg( 1, sizeof(cl_int3), &size );
	mKernWriteMetaballs->setKernelArg( 2, mMetaballPositions );
	mKernWriteMetaballs->setKernelArg( 3, sizeof(cl_int), &mNumBalls );
	
	size_t work = width * height * depth;
	mCommandQueue->NDRangeKernel( mKernWriteMetaballs, 1, nullptr, &work, nullptr );
	
	/* End */
	
//	int zero = 0;
//	kernel_memory_write(vertex_index, sizeof(int), &zero);
//	
//	const int num_workers = (width-1) * (height-1) * (depth-1);
//	
//	kernel_set_argument(construct_surface, 0, sizeof(kernel_memory), &volume);
//	kernel_set_argument(construct_surface, 1, sizeof(cl_int3), &size);
//	kernel_set_argument(construct_surface, 2, sizeof(kernel_memory), &vertex_positions_buffer);
//	kernel_set_argument(construct_surface, 3, sizeof(kernel_memory), &vertex_index);
//	kernel_run(construct_surface, num_workers);
//	
//	kernel_memory_read(vertex_index, sizeof(cl_int), &num_verts);
//	
//	/* Generate Normals */
//	
//	if (num_verts > 0) {
//	  kernel_set_argument(generate_normals_smooth, 0, sizeof(kernel_memory), &vertex_positions_buffer);
//	  kernel_set_argument(generate_normals_smooth, 1, sizeof(kernel_memory), &vertex_normals_buffer);
//	  kernel_set_argument(generate_normals_smooth, 2, sizeof(kernel_memory), &metaball_positions);
//	  kernel_set_argument(generate_normals_smooth, 3, sizeof(cl_int), &num_metaballs);
//	  kernel_run(generate_normals_smooth, num_verts);
//	}
	
	/*
	kernel_set_argument(generate_normals, 0, sizeof(kernel_memory), &vertex_positions_buffer);
	kernel_set_argument(generate_normals, 1, sizeof(kernel_memory), &vertex_normals_buffer);
	kernel_run(generate_normals, num_verts/3);
	*/
	
//	kernel_memory_gl_release(vertex_positions_buffer);
//	kernel_memory_gl_release(vertex_normals_buffer);
//	kernel_memory_gl_release(metaball_positions);
//	
//	kernel_run_finish();

}

