#pragma once

#include <clover2/pose/data/frame_data.hpp>
#include <clover2/pose/graph/edge_pose.hpp>
#include <clover2/pose/graph/id_manager.hpp>
#include <clover2/pose/graph/vertex_pose.hpp>

#include <g2o/core/block_solver.h>
#include <g2o/core/optimization_algorithm_levenberg.h>
#include <g2o/core/sparse_optimizer.h>
#include <g2o/solvers/dense/linear_solver_dense.h>

#include <Eigen/Geometry>

#include <mutex>
#include <unordered_map>

namespace clover2::pose::graph {

class graph_manager {
public:
    graph_manager();
    ~graph_manager();

    void add_frame(const data::frame_data& frame);
    void optimize(int iterations = 10);
    Eigen::Isometry3d get_current_pose() const;

private:
    void setup_optimizer();
    void add_marker_vertex(int32_t marker_id, const Eigen::Isometry3d& pose);

    using BlockSolver =
        g2o::BlockSolver<g2o::BlockSolverTraits<6, 6>>;
    using LinearSolver =
        g2o::LinearSolverDense<BlockSolver::PoseMatrixType>;

    g2o::SparseOptimizer m_optimizer;
    id_manager m_id_manager;

    std::unordered_map<int32_t, vertex_pose*> m_pose_vertices;
    std::unordered_map<int32_t, vertex_pose*> m_marker_vertices;

    int32_t m_current_camera_id = -1;
    mutable std::mutex m_mutex;
};

}  // namespace clover2::pose::graph
