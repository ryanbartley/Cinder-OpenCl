//
//  MarchingCubes.cpp
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#include "MarchingCubes.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace ci::gl;
using namespace ci::cl;

MarchingCubes::MarchingCubes()
{
	const int full_size = width * height * depth;

	std::vector<Vec4f> pointData(full_size);
	

	int x, y, z;
	for(x = 0; x < width; x++)
	for(y = 0; y < height; y++)
	for(z = 0; z < depth; z++) {
		int id = x + y * width + z * width * height;
		Vec4f position = Vec4f(x, y, z, 1);
		pointData[id] = position;
	}

	mGlPointPositions = gl::Vbo::create(GL_ARRAY_BUFFER,
										sizeof(Vec4f) * full_size,
										pointData.data(),
										GL_STATIC_DRAW);
	
	std::vector<Vec4f> pointColorData(full_size);
	memset( pointColorData.data(), 0, sizeof(Vec4f) * full_size );
	mGlPointColors = gl::Vbo::create(GL_ARRAY_BUFFER,
									 sizeof(Vec4f) * full_size,
									 pointColorData.data(),
									 GL_DYNAMIC_COPY);
	
	mClPointColors = cl::BufferObj::create( mGlPointColors, CL_MEM_READ_WRITE );
	
	mClVolume = cl::BufferObj::create( CL_MEM_READ_WRITE, sizeof(float) * full_size, nullptr );
	
	std::vector<Vec4f> vertPosData(sizeof(Vec4f) * MAX_VERTS);
	memset(vertPosData.data(), 0, sizeof(Vec4f) * MAX_VERTS);
	mGlVertPositions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(Vec4f) * MAX_VERTS, vertPosData.data(), GL_DYNAMIC_COPY);

	mClVertPositions = cl::BufferObj::create( mGlVertPositions, CL_MEM_READ_WRITE );
	
	std::vector<Vec4f> vertNormData(sizeof(Vec4f) * MAX_VERTS);
	memset(vertNormData.data(), 0, sizeof(Vec4f) * MAX_VERTS);
	mGlVertNormals = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(Vec4f) * MAX_VERTS, vertNormData.data(), GL_DYNAMIC_COPY);
	
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
	
}



