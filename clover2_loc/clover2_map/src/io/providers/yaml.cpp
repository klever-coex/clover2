#include <clover2_map/io/providers/yaml.hpp>

// ROS2
#include <rclcpp/logging.hpp>

// tf2
#include <tf2/LinearMath/Matrix3x3.hpp>
#include <tf2/LinearMath/Quaternion.hpp>

// yaml
#include <yaml-cpp/yaml.h>

// STL
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace YAML {

template <>
struct convert<Eigen::Isometry3d> {
    static bool decode(const Node& node, Eigen::Isometry3d& pose) {
        pose.translation() = Eigen::Vector3d(node["x"].as<double>(0.0),
                                             node["y"].as<double>(0.0),
                                             node["z"].as<double>(0.0));

        tf2::Quaternion q;
        if (node["rot"]) {
            q.setRPY(  //
                node["rot"]["roll"].as<double>(0.0),
                node["rot"]["pitch"].as<double>(0.0),
                node["rot"]["yaw"].as<double>(0.0));
        } else if (node["quat"]) {
            q.setValue(  //
                node["quat"]["x"].as<double>(0.0),
                node["quat"]["y"].as<double>(0.0),
                node["quat"]["z"].as<double>(0.0),
                node["quat"]["w"].as<double>(1.0));
        } else {
            q.setRPY(0.0, 0.0, 0.0);
        }

        pose.linear() =
            Eigen::Quaterniond(q.w(), q.x(), q.y(), q.z()).toRotationMatrix();

        return true;
    }

    static Node encode(const Eigen::Isometry3d& pose) {
        const auto& t = pose.translation();

        Node node;
        node["x"] = t.x();
        node["y"] = t.y();
        node["z"] = t.z();

        Eigen::Quaterniond eq(pose.linear());
        tf2::Quaternion q(eq.x(), eq.y(), eq.z(), eq.w());
        double roll = 0.0;
        double pitch = 0.0;
        double yaw = 0.0;
        tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);

        Node rot;
        rot["roll"] = roll;
        rot["pitch"] = pitch;
        rot["yaw"] = yaw;
        node["rot"] = rot;

        return node;
    }
};

template <>
struct convert<clover2_map::marker> {
    static bool decode(const Node& node, clover2_map::marker& marker) {
        if (!node.IsMap()) {
            return false;
        }

        if (!node["id"]) {
            throw std::runtime_error("Marker id is required");
        }

        marker.id = node["id"].as<int>();
        marker.size = node["size"].as<double>(-1.0);
        marker.marker_frame_id = node["frame_id"].as<std::string>("");

        if (node["type"]) {
            marker.type = clover2_map::marker_type::from_string(
                node["type"].as<std::string>());
        }

        Eigen::Isometry3d pose = Eigen::Isometry3d::Identity();
        if (node["pose"]) {
            pose = node["pose"].as<Eigen::Isometry3d>();
        }

        if (marker.type == clover2_map::marker_type::fixed) {
            marker.pose = pose;
        }

        return true;
    }

    static Node encode(const clover2_map::marker& marker) {
        Node node;
        node["id"] = marker.id;
        node["size"] = marker.size;

        if (marker.type != clover2_map::marker_type::fixed) {
            node["type"] = marker.type.to_string();
        }

        if (marker.pose) {
            node["pose"] = *marker.pose;
        }

        if (!marker.marker_frame_id.empty()) {
            node["frame_id"] = marker.marker_frame_id;
        }

        return node;
    }
};

}  // namespace YAML

namespace clover2_map::io::providers {

yaml::yaml(const std::filesystem::path& filename, rclcpp::Logger logger)
    : base_provider(filename, logger) {}

void yaml::load(clover2_map::map& map) {
    try {
        load_impl(map);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse '" + m_filename.string() +
                                 "': " + e.what());
    }
}

void yaml::save(const clover2_map::map& map) const {
    YAML::Node root;
    root["name"] = map.name;
    root["frame_id"] = map.frame_id;
    root["dict"] = map.dictionary.empty() ? "4X4_1000" : map.dictionary;

    double common_size = -1.0;
    if (!map.markers.empty()) {
        common_size = map.markers[0].size;
        for (const auto& m : map.markers) {
            if (std::abs(m.size - common_size) > 1e-9) {
                common_size = -1.0;
                break;
            }
        }
    }

    if (common_size > 0.0) {
        root["default_size"] = common_size;
    }

    YAML::Node markers(YAML::NodeType::Sequence);
    for (const auto& m : map.markers) {
        YAML::Node entry = YAML::convert<clover2_map::marker>::encode(m);

        if (common_size > 0.0) {
            entry.remove("size");
        }

        markers.push_back(entry);
    }

    root["markers"] = markers;

    std::ofstream out(m_filename);
    if (!out.good()) {
        throw std::runtime_error(
            "Unable to open for write: " + m_filename.string() +
            ", reason: " + strerror(errno));
    }

    out << root;
}

void yaml::load_impl(clover2_map::map& map) {
    YAML::Node config = YAML::LoadFile(m_filename.string());

    map.name = config["name"].as<std::string>(m_filename.stem().string());
    map.frame_id = config["frame_id"].as<std::string>("map");
    map.version = config["version"].as<int>(0);
    map.dictionary = config["dict"].as<std::string>("4X4_1000");

    RCLCPP_DEBUG(m_logger, "Parsing map '%s' from '%s'", map.name.c_str(),
                 m_filename.string().c_str());

    double default_size = config["default_size"].as<double>(-1.0);

    if (!config["markers"].IsSequence()) {
        throw std::runtime_error("Map is empty");
    }

    std::unordered_set<int> ids;
    for (const auto& it : config["markers"]) {
        auto mk = it.as<clover2_map::marker>();

        if (mk.type != clover2_map::marker_type::fixed && it["pose"]) {
            RCLCPP_WARN(m_logger, "Pose of %s marker %d in '%s' is ignored",
                        mk.type.to_string().c_str(), mk.id,
                        m_filename.string().c_str());
        }

        if (mk.size < 0.0 && default_size < 0.0) {
            RCLCPP_ERROR(m_logger,
                         "Missing size property for %d marker. Default "
                         "size also dont exist.",
                         mk.id);
            throw std::runtime_error("Missing marker size");
        }

        if (mk.size < 0.0) {
            mk.size = default_size;
        }

        if (mk.marker_frame_id.empty()) {
            mk.marker_frame_id =
                map.frame_id + "_aruco_" + std::to_string(mk.id);
        }

        if (!ids.insert(mk.id).second) {
            throw std::runtime_error("Duplicate marker id " +
                                     std::to_string(mk.id) + " in '" +
                                     m_filename.string() + "'");
        }

        map.markers.push_back(std::move(mk));
    }
}

}  // namespace clover2_map::io::providers
