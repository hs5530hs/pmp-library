//=============================================================================
// Copyright (C) 2011-2019 The pmp-library developers
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//=============================================================================
#pragma once
//=============================================================================

#include <pmp/SurfaceMesh.h>

#include <pmp/algorithms/Heap.h>
#include <pmp/algorithms/NormalCone.h>
#include <pmp/algorithms/Quadric.h>

#include <set>
#include <vector>

//=============================================================================

namespace pmp {

//=============================================================================
//! \addtogroup algorithms algorithms
//! @{
//=============================================================================

//! \brief Surface mesh simplification based on approximation error and fairness criteria.
//! \details Performs incremental greedy mesh simplification based on halfedge
//! collapses. See \cite kobbelt_1998_general and \cite garland_1997_surface for
//! details.
class SurfaceSimplification
{
public:
    // constructor
    SurfaceSimplification(SurfaceMesh& mesh);

    // destructor
    ~SurfaceSimplification();

    //! initialize decimater
    void initialize(Scalar aspect_ratio = 0.0, Scalar edge_length = 0.0,
                    unsigned int max_valence = 0, Scalar normal_deviation = 0.0,
                    Scalar hausdorff_error = 0.0);

    //! decimate down to n vertices
    void simplify(unsigned int n_vertices);

private: //------------------------------------------------------ private types
    //! Store data for an halfedge collapse
    /*
                vl
                *
               / \
              /   \
             / fl  \
         v0 *------>* v1
             \ fr  /
              \   /
               \ /
                *
                vr
    */
    struct CollapseData
    {
        CollapseData(SurfaceMesh& sm, SurfaceMesh::Halfedge h);

        SurfaceMesh& mesh;

        SurfaceMesh::Halfedge v0v1; // Halfedge to be collapsed
        SurfaceMesh::Halfedge v1v0; // Reverse halfedge
        SurfaceMesh::Vertex v0;     // Vertex to be removed
        SurfaceMesh::Vertex v1;     // Remaining vertex
        SurfaceMesh::Face fl;       // Left face
        SurfaceMesh::Face fr;       // Right face
        SurfaceMesh::Vertex vl;     // Left vertex
        SurfaceMesh::Vertex vr;     // Right vertex
        SurfaceMesh::Halfedge v1vl, vlv0, v0vr, vrv1;
    };

    //! Heap interface
    class HeapInterface
    {
    public:
        HeapInterface(SurfaceMesh::VertexProperty<float> prio,
                      SurfaceMesh::VertexProperty<int> pos)
            : prio_(prio), pos_(pos)
        {
        }

        bool less(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1)
        {
            return prio_[v0] < prio_[v1];
        }
        bool greater(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1)
        {
            return prio_[v0] > prio_[v1];
        }
        int get_heap_position(SurfaceMesh::Vertex v) { return pos_[v]; }
        void set_heap_position(SurfaceMesh::Vertex v, int pos) { pos_[v] = pos; }

    private:
        SurfaceMesh::VertexProperty<float> prio_;
        SurfaceMesh::VertexProperty<int> pos_;
    };

    typedef Heap<SurfaceMesh::Vertex, HeapInterface> PriorityQueue;

    typedef std::vector<Point> Points;

private: //-------------------------------------------------- private functions
    // put the vertex v in the priority queue
    void enqueue_vertex(SurfaceMesh::Vertex v);

    // is collapsing the halfedge h allowed?
    bool is_collapse_legal(const CollapseData& cd);

    // what is the priority of collapsing the halfedge h
    float priority(const CollapseData& cd);

    // postprocess halfedge collapse
    void postprocess_collapse(const CollapseData& cd);

    // compute aspect ratio for face f
    Scalar aspect_ratio(SurfaceMesh::Face f) const;

    // compute distance from p to triagle f
    Scalar distance(SurfaceMesh::Face f, const Point& p) const;

private: //------------------------------------------------------- private data
    SurfaceMesh& mesh_;

    bool initialized_;

    SurfaceMesh::VertexProperty<float> vpriority_;
    SurfaceMesh::VertexProperty<SurfaceMesh::Halfedge> vtarget_;
    SurfaceMesh::VertexProperty<int> heap_pos_;
    SurfaceMesh::VertexProperty<Quadric> vquadric_;
    SurfaceMesh::FaceProperty<NormalCone> normal_cone_;
    SurfaceMesh::FaceProperty<Points> face_points_;

    SurfaceMesh::VertexProperty<Point> vpoint_;
    SurfaceMesh::FaceProperty<Point> fnormal_;
    SurfaceMesh::VertexProperty<bool> vselected_;
    SurfaceMesh::VertexProperty<bool> vfeature_;
    SurfaceMesh::EdgeProperty<bool> efeature_;

    PriorityQueue* queue_;

    bool has_selection_;
    bool has_features_;
    Scalar normal_deviation_;
    Scalar hausdorff_error_;
    Scalar aspect_ratio_;
    Scalar edge_length_;
    unsigned int max_valence_;
};

//=============================================================================
//! @}
//=============================================================================
} // namespace pmp
//=============================================================================
