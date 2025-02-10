/**
 * @file    loop_mesh_builder.cpp
 *
 * @author Jakub Vlk <xvlkja07@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP loops
 *
 * @date    29.11.2024
 **/

#include <iostream>
#include <math.h>
#include <limits>
#include <cstring>

#include "loop_mesh_builder.h"

LoopMeshBuilder::LoopMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "OpenMP Loop")
{
    int num_threads = omp_get_max_threads();

    for (int i = 0; i < num_threads; i++)
    {
        mTriangles_partical.emplace_back();
        mTriangles_partical[i].reserve(8192*2);
    }
}

unsigned LoopMeshBuilder::marchCubes(const ParametricScalarField& field)
{
    const size_t totalCubesCount = mGridSize * mGridSize * mGridSize;

    unsigned totalTriangles = 0;

    unsigned cTriangles = 0;

#pragma omp parallel default(none) shared(totalCubesCount, field, cTriangles, mTriangles, totalTriangles, std::cout)
    {
#pragma omp for schedule(dynamic, 64)
        for (size_t i = 0; i < totalCubesCount; ++i)
        {
            // 3. Compute 3D position in the grid.
            Vec3_t<float> cubeOffset(i % mGridSize,
                                     (i / mGridSize) % mGridSize,
                                     i / (mGridSize * mGridSize));

            // 4. Evaluate "Marching Cube" at given position in the grid and
            //    store the number of triangles generated.
            buildCube(cubeOffset, field);
        }

#pragma omp barrier

        int thid = omp_get_thread_num();
#pragma omp atomic
        totalTriangles += mTriangles_partical[thid].size();
    }

    return totalTriangles;
}

float LoopMeshBuilder::evaluateFieldAt(const Vec3_t<float>& pos, const ParametricScalarField& field)
{
    // NOTE: This method is called from "buildCube(...)"!

    // 1. Store pointer to and number of 3D points in the field
    //    (to avoid "data()" and "size()" call in the loop).
    const Vec3_t<float>* pPoints = field.getPoints().data();
    const unsigned count = unsigned(field.getPoints().size());

    float value = std::numeric_limits<float>::max();

    // 2. Find minimum square distance from points "pos" to any point in the
    //    field.
    // #pragma omp simd reduction(min:value)
    for (unsigned i = 0; i < count; ++i)
    {
        {
            float distanceSquared = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
            distanceSquared += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
            distanceSquared += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

            // Comparing squares instead of real distance to avoid unnecessary
            // "sqrt"s in the loop.
            value = std::min(value, distanceSquared);
        }
    }

    // 3. Finally take square root of the minimal square distance to get the real distance
    return sqrt(value);
}

void LoopMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t& triangle)
{
    int thread_id = omp_get_thread_num();

    mTriangles_partical[thread_id].push_back(triangle);
}
