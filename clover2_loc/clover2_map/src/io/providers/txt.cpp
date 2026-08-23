#include <clover2_map/io/providers/txt.hpp>

// ROS2
#include <rclcpp/logging.hpp>

// tf2
#include <tf2/LinearMath/Matrix3x3.hpp>
#include <tf2/LinearMath/Quaternion.hpp>

// STL
#include <cctype>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace clover2_map::io::providers {

txt::txt(const std::filesystem::path& filename, rclcpp::Logger logger)
    : base_provider(filename, logger) {}

void txt::load(clover2_map::map& map) {
    try {
        load_impl(map);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse '" + m_filename.string() +
                                 "': " + e.what());
    }
}

void txt::save(const clover2_map::map& map) const {
    std::ofstream out(m_filename);
    if (!out.good()) {
        throw std::runtime_error(
            "Unable to open for write: " + m_filename.string() +
            ", reason: " + strerror(errno));
    }

    out << std::setprecision(12);
    for (const auto& m : map.markers) {
        if (!m.pose) {
            RCLCPP_DEBUG(m_logger,
                         "Skipping marker %d without pose when saving '%s' as "
                         "txt",
                         m.id, m_filename.string().c_str());
            continue;
        }

        Eigen::Quaterniond eq(m.pose->linear());
        tf2::Quaternion q(eq.x(), eq.y(), eq.z(), eq.w());
        double roll = 0.0;
        double pitch = 0.0;
        double yaw = 0.0;
        tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);

        const auto& t = m.pose->translation();
        out << m.id << ' ' << m.size << ' ' << t.x() << ' ' << t.y() << ' '
            << t.z() << ' ' << yaw << ' ' << pitch << ' ' << roll << '\n';
    }
}

void txt::load_impl(clover2_map::map& map) {
    map.name = m_filename.stem().string();
    map.frame_id = "map";

    std::ifstream f(m_filename);
    if (!f.good()) {
        throw std::runtime_error("Unable to open " + m_filename.string() +
                                 ", reason: " + strerror(errno));
    }

    std::unordered_map<int, int> marker_lines;

    std::string line;
    int line_no = 0;
    while (std::getline(f, line)) {
        line_no++;

        clover2_map::marker marker;
        marker.type = clover2_map::marker_type::fixed;

        double yaw = 0.0;
        double pitch = 0.0;
        double roll = 0.0;

        std::istringstream s(line);

        char first = 0;
        if (!(s >> first) || first == '#') {
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(first))) {
            s.putback(first);
        } else {
            throw std::runtime_error("Malformed input: " + line);
        }

        Eigen::Vector3d position = Eigen::Vector3d::Zero();
        if (!(s >> marker.id >> marker.size >> position.x() >> position.y())) {
            RCLCPP_ERROR(
                m_logger,
                "Not enough data in line: %s; "
                "Each marker must have at least id, length, x, y fields",
                line.c_str());
            continue;
        }

        if (!(s >> position.z())) {
            RCLCPP_DEBUG(m_logger,
                         "No z coordinate provided for marker %d, assuming 0",
                         marker.id);
        }

        if (!(s >> yaw)) {
            RCLCPP_DEBUG(m_logger, "No yaw provided for marker %d, assuming 0",
                         marker.id);
        }

        if (!(s >> pitch)) {
            RCLCPP_DEBUG(m_logger, "No pitch provided for marker %d, assuming 0",
                         marker.id);
        }

        if (!(s >> roll)) {
            RCLCPP_DEBUG(m_logger, "No roll provided for marker %d, assuming 0",
                         marker.id);
        }

        marker.marker_frame_id =
            map.frame_id + "_aruco_" + std::to_string(marker.id);

        tf2::Quaternion q;
        q.setRPY(roll, pitch, yaw);

        marker.pose = Eigen::Isometry3d::Identity();
        marker.pose->translation() = position;
        marker.pose->linear() =
            Eigen::Quaterniond(q.w(), q.x(), q.y(), q.z()).toRotationMatrix();

        auto [_, inserted] = marker_lines.try_emplace(marker.id, line_no);
        if (!inserted) {
            throw std::runtime_error(
                "Duplicate marker id " + std::to_string(marker.id) + " in '" +
                m_filename.string() + "' (lines " +
                std::to_string(marker_lines.at(marker.id)) + " and " +
                std::to_string(line_no) + ")");
        }

        map.markers.push_back(std::move(marker));
    }
}

}  // namespace clover2_map::io::providers
