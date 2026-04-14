#include <clover2/pose/graph/graph_manager.hpp>

#include <rclcpp/logging.hpp>

namespace clover2::pose::graph {

graph_manager::graph_manager() { setup_optimizer(); }

graph_manager::~graph_manager() {
    m_optimizer.clear();
}

void graph_manager::setup_optimizer() {
    auto linear_solver = std::make_unique<LinearSolver>();
    auto block_solver =
        std::make_unique<BlockSolver>(std::move(linear_solver));
    auto algorithm =
        new g2o::OptimizationAlgorithmLevenberg(std::move(block_solver));
    m_optimizer.setAlgorithm(algorithm);
    m_optimizer.setVerbose(false);
}

void graph_manager::add_marker_vertex(int32_t marker_id,
                                      const Eigen::Isometry3d& pose) {
    if (m_marker_vertices.count(marker_id) != 0) {
        return;
    }

    auto* vertex = new vertex_pose();
    vertex->setId(m_id_manager.next_landmark_id());
    vertex->setEstimate(pose);
    vertex->setFixed(true);
    m_optimizer.addVertex(vertex);
    m_marker_vertices[marker_id] = vertex;
    m_id_manager.register_marker(marker_id, vertex->id());
}

void graph_manager::add_frame(const data::frame_data& frame) {
    std::lock_guard<std::mutex> lock(m_mutex);

    int32_t camera_id = m_id_manager.next_pose_id();
    auto* cam_vertex = new vertex_pose();
    cam_vertex->setId(camera_id);
    cam_vertex->setEstimate(m_current_camera_id >= 0
                                ? m_pose_vertices.at(m_current_camera_id)
                                      ->estimate()
                                : Eigen::Isometry3d::Identity());
    cam_vertex->setFixed(m_current_camera_id < 0);
    m_optimizer.addVertex(cam_vertex);
    m_pose_vertices[camera_id] = cam_vertex;
    m_current_camera_id = camera_id;

    for (const auto& obs : frame.observations) {
        if (obs.obs_type != data::observation::type::marker_pose) {
            continue;
        }

        Eigen::Isometry3d marker_pose = obs.landmark_world;

        add_marker_vertex(obs.id, marker_pose);

        auto it = m_marker_vertices.find(obs.id);
        if (it == m_marker_vertices.end()) {
            continue;
        }

        Eigen::MatrixXd info = obs.information.rows() == 6 && obs.information.cols() == 6
                                   ? obs.information
                                   : Eigen::Matrix<double, 6, 6>::Identity();

        auto* edge = new edge_pose();
        edge->setVertex(0, cam_vertex);
        edge->setVertex(1, it->second);
        edge->setMeasurement(obs.pose);
        edge->setInformation(info);
        m_optimizer.addEdge(edge);
    }
}

void graph_manager::optimize(int iterations) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_current_camera_id < 0) {
        return;
    }
    m_optimizer.initializeOptimization();
    m_optimizer.optimize(iterations);
}

Eigen::Isometry3d graph_manager::get_current_pose() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_current_camera_id < 0) {
        return Eigen::Isometry3d::Identity();
    }
    auto it = m_pose_vertices.find(m_current_camera_id);
    if (it == m_pose_vertices.end()) {
        return Eigen::Isometry3d::Identity();
    }
    return it->second->estimate();
}

}  // namespace clover2::pose::graph
