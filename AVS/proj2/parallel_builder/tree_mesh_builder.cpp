/**
 * @file    tree_mesh_builder.cpp
 *
 * @author Jakub Vlk <xvlkja07@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    29.11.2024
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "tree_mesh_builder.h"
#include <omp.h>

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{
    int num_threads = omp_get_max_threads();

    for (int i = 0; i < num_threads; i++)
    {
        mTriangles_partical.emplace_back();
        mTriangles_partical[i].reserve(8192*2);
    }
}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField& field)
{
    // Suggested approach to tackle this problem is to add new method to
    // this class. This method will call itself to process the children.
    // It is also strongly suggested to first implement Octree as sequential
    // code and only when that works add OpenMP tasks to achieve parallelism.

    Vec3_t<float> p = {(mGridSize / 2.0f) * mGridResolution};
    size_t totalTriangles = 0;
#pragma omp parallel default(none) shared(totalTriangles, field, p)
    {
#pragma omp single
        {

                decompozicion(field, {0, 0, 0}, mGridSize);
        }
    }

    for (int i = 0; i < omp_get_max_threads(); i++)
    {
        totalTriangles += mTriangles_partical[i].size();
    }

    return totalTriangles;
}

/**
 * @brief Just notest for me:
 *
 * 1) param p is necesary for center of the block (f(p) the p is the center of the block)
 * 2) l that is the level in every recursion is cut in half
 * 3) and a that is the edge of the block is also cut in half
 * no need) coordanitase of the block are from p - a/2 to p + a/2
 * @param p center of the block (f(p) the p is the center of the block)
 * @param l
 * @param e
 * @param field
 * @param depth
 * @return
 */
void TreeMeshBuilder::decompozicion(const ParametricScalarField& field, const Vec3_t<float>& cubeOffset,
                                    const unsigned cubeSize)
{
    const float subCubeSize = cubeSize / 2.F;
    const Vec3_t<float> midPoint(
        (cubeOffset.x + subCubeSize) * mGridResolution,
        (cubeOffset.y + subCubeSize) * mGridResolution,
        (cubeOffset.z + subCubeSize) * mGridResolution
    );

    float value = evaluateFieldAt(midPoint, field);
    const float x = mIsoLevel + sqrt(3.f) / 2.f * cubeSize * mGridResolution;
    if (value > x)
    {
        return;
    }

    if (cubeSize <= 1)
    {
        buildCube(cubeOffset, field);
        return;
    }


    // split by 8
    for (int i = 0; i < 8; i++)
    {
#pragma omp task default(none) shared(field, cubeOffset, cubeSize, subCubeSize) firstprivate(i) if (cubeSize > 8)
        {
            Vec3_t<float> subCubeOffset = cubeOffset;
            if (i & 1) subCubeOffset.x += subCubeSize;
            if (i & 2) subCubeOffset.y += subCubeSize;
            if (i & 4) subCubeOffset.z += subCubeSize;

            const unsigned subCubeSizeUns = cubeSize / 2;


            decompozicion(field, subCubeOffset, subCubeSizeUns);
        }
    }
#pragma omp taskwait
    return;
}


float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float>& pos, const ParametricScalarField& field)
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

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t& triangle)
{
    int thread_id = omp_get_thread_num();
    mTriangles_partical[thread_id].push_back(triangle);
}
