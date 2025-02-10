/**
 * @file    tree_mesh_builder.h
 *
 * @author Jakub Vlk <xvlkja07@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    29.11.2024
 **/

#ifndef TREE_MESH_BUILDER_H
#define TREE_MESH_BUILDER_H

#include "base_mesh_builder.h"
#include <omp.h>

class TreeMeshBuilder : public BaseMeshBuilder
{
public:
    TreeMeshBuilder(unsigned gridEdgeSize);

protected:
    unsigned marchCubes(const ParametricScalarField& field);

    float evaluateFieldAt(const Vec3_t<float>& pos, const ParametricScalarField& field);

    void emitTriangle(const Triangle_t& triangle);

    void decompozicion(const ParametricScalarField& field, const Vec3_t<float>& cubeOffset, const unsigned cubeSize);

    const Triangle_t* getTrianglesArray() const;

    std::vector<std::vector<Triangle_t>> mTriangles_partical;
    mutable std::vector<Triangle_t> mTriangles;

    float closesPointsDistance(const Vec3_t<float>& p) const;
};

inline const BaseMeshBuilder::Triangle_t* TreeMeshBuilder::getTrianglesArray() const
{
    static bool first = true;
    if (first)
    {
        first = false;
        for (int i = 0; i < omp_get_max_threads(); i++)
        {
            int thid = i;
            mTriangles.insert(
                mTriangles.end(),
                std::make_move_iterator(mTriangles_partical[thid].begin()),
                std::make_move_iterator(mTriangles_partical[thid].end())
            );
        }
    }
    return mTriangles.data();
}

#endif // TREE_MESH_BUILDER_H
