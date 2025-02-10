/**
 * @file    loop_mesh_builder.h
 *
 * @author Jakub Vlk <xvlkja07@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP loops
 *
 * @date    29.11.2024
 **/

#ifndef LOOP_MESH_BUILDER_H
#define LOOP_MESH_BUILDER_H

#include <vector>
#include "base_mesh_builder.h"
#include "omp.h"

class LoopMeshBuilder : public BaseMeshBuilder
{
public:
    LoopMeshBuilder(unsigned gridEdgeSize);

protected:
    unsigned marchCubes(const ParametricScalarField& field);

    float evaluateFieldAt(const Vec3_t<float>& pos, const ParametricScalarField& field);

    void emitTriangle(const Triangle_t& triangle);

    // const Triangle_t *getTrianglesArray() const { return nullptr; }

    const Triangle_t* getTrianglesArray() const override;


    std::vector<std::vector<Triangle_t>> mTriangles_partical; ///< Temporary array of triangles
    mutable std::vector<Triangle_t> mTriangles;
};

inline const BaseMeshBuilder::Triangle_t* LoopMeshBuilder::getTrianglesArray() const
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

#endif // LOOP_MESH_BUILDER_H
