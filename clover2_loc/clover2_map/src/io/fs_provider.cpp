#include <clover2/map/io/fs_provider.hpp>
#include <rclcpp/logging.hpp>
#include <tf2/LinearMath/Matrix3x3.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <yaml-cpp/yaml.h>

#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace YAML {
template <>
struct convert<clover2_pose_msgs::msg::Marker> {
    static bool decode(const Node& node,
                       clover2_pose_msgs::msg::Marker& marker) {
        if (!node.IsMap()) {
            return false;
        }

        if (!node["id"]) {
            throw std::runtime_error("Marker id is required");
        }

        marker.id = node["id"].as<int>();
        marker.size = node["size"].as<double>(-1.0);

        marker.pose.pose.position.x = 0.0;
        marker.pose.pose.position.y = 0.0;
        marker.pose.pose.position.z = 0.0;

        if (node["pose"]) {
            auto& pose = node["pose"];

            marker.pose.pose.position.x = pose["x"].as<double>(0.0);
            marker.pose.pose.position.y = pose["y"].as<double>(0.0);
            marker.pose.pose.position.z = pose["z"].as<double>(0.0);
        }

        tf2::Quaternion q;
        if (node["rot"]) {
            auto& rot = node["rot"];

            q.setRPY(                          //
                rot["roll"].as<double>(0.0),   //
                rot["pitch"].as<double>(0.0),  //
                rot["yaw"].as<double>(0.0)     //
            );
        } else if (node["quat"]) {
            auto& quat = node["quat"];

            q.setValue(                     //
                quat["x"].as<double>(0.0),  //
                quat["y"].as<double>(0.0),  //
                quat["z"].as<double>(0.0),  //
                quat["w"].as<double>(1.0)   //
            );
        } else {
            q.setRPY(0.0, 0.0, 0.0);
        }

        marker.pose.pose.orientation.x = q.x();
        marker.pose.pose.orientation.y = q.y();
        marker.pose.pose.orientation.z = q.z();
        marker.pose.pose.orientation.w = q.w();

        marker.marker_frame_id = node["frame_id"].as<std::string>("");

        return true;
    }

    static Node encode(const clover2_pose_msgs::msg::Marker& marker) {
        Node node;
        node["id"] = static_cast<int>(marker.id);
        node["size"] = static_cast<double>(marker.size);

        Node pose;
        pose["x"] = marker.pose.pose.position.x;
        pose["y"] = marker.pose.pose.position.y;
        pose["z"] = marker.pose.pose.position.z;
        node["pose"] = pose;

        tf2::Quaternion q(
            marker.pose.pose.orientation.x, marker.pose.pose.orientation.y,
            marker.pose.pose.orientation.z, marker.pose.pose.orientation.w);
        double roll = 0.0;
        double pitch = 0.0;
        double yaw = 0.0;
        tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);

        Node rot;
        rot["roll"] = roll;
        rot["pitch"] = pitch;
        rot["yaw"] = yaw;
        node["rot"] = rot;

        if (!marker.marker_frame_id.empty()) {
            node["frame_id"] = marker.marker_frame_id;
        }

        return node;
    }
};

}  // namespace YAML

namespace clover2::map::io {

namespace {

void load_yaml(const rclcpp::Logger& logger,
               const std::filesystem::path& filename,
               clover2_pose_msgs::msg::MarkerMap& map) {
    YAML::Node config = YAML::LoadFile(filename.string());
    map.name = config["name"].as<std::string>(filename.stem());

    RCLCPP_DEBUG(logger, "Parsing map '%s' from '%s'", map.name.c_str(),
                 filename.c_str());

    double default_size = config["default_size"].as<double>(-1.0);
    map.header.frame_id = config["frame_id"].as<std::string>("map");

    if (config["markers"].IsSequence()) {
        for (const auto& it : config["markers"]) {
            auto marker = it.as<clover2_pose_msgs::msg::Marker>();

            if (marker.size < 0.0 && default_size < 0.0) {
                RCLCPP_ERROR(logger,
                             "Missing size property for %d marker. Default "
                             "size also dont exist.",
                             marker.id);
                throw std::runtime_error("Missing marker size");
            }

            if (marker.size < 0.0) {
                marker.size = static_cast<float>(default_size);
            }

            if (marker.marker_frame_id.empty()) {
                marker.marker_frame_id =
                    map.header.frame_id + "_aruco_" + std::to_string(marker.id);
            }

            map.markers.push_back(marker);
        }
    } else {
        throw std::runtime_error("Map is empty");
    }
}

void load_txt(const rclcpp::Logger& logger,
              const std::filesystem::path& filename,
              clover2_pose_msgs::msg::MarkerMap& map) {
    map.name = filename.stem();
    map.header.frame_id = map.name;

    std::ifstream f(filename);
    if (!f.good()) {
        throw std::runtime_error("Unable to open " + filename.string() +
                                 ", reason: " + strerror(errno));
    }

    std::string line;
    while (std::getline(f, line)) {
        clover2_pose_msgs::msg::Marker marker;
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

        if (!(s >> marker.id >> marker.size >> marker.pose.pose.position.x >>
              marker.pose.pose.position.y)) {
            RCLCPP_ERROR(
                logger,
                "Not enough data in line: %s; "
                "Each marker must have at least id, length, x, y fields",
                line.c_str());
            continue;
        }

        if (!(s >> marker.pose.pose.position.z)) {
            RCLCPP_DEBUG(logger,
                         "No z coordinate provided for marker %d, assuming 0",
                         marker.id);
            marker.pose.pose.position.z = 0.0;
        }

        if (!(s >> yaw)) {
            RCLCPP_DEBUG(logger, "No yaw provided for marker %d, assuming 0",
                         marker.id);
            yaw = 0.0;
        }

        if (!(s >> pitch)) {
            RCLCPP_DEBUG(logger, "No pitch provided for marker %d, assuming 0",
                         marker.id);
            pitch = 0.0;
        }

        if (!(s >> roll)) {
            RCLCPP_DEBUG(logger, "No roll provided for marker %d, assuming 0",
                         marker.id);
            roll = 0.0;
        }

        marker.marker_frame_id =
            map.header.frame_id + "_aruco_" + std::to_string(marker.id);

        tf2::Quaternion q;
        q.setRPY(roll, pitch, yaw);

        marker.pose.pose.orientation.x = q.x();
        marker.pose.pose.orientation.y = q.y();
        marker.pose.pose.orientation.z = q.z();
        marker.pose.pose.orientation.w = q.w();

        map.markers.push_back(marker);
    }

    map.header.frame_id = "map";
}

void save_yaml(const std::filesystem::path& filename,
               const clover2_pose_msgs::msg::MarkerMap& map) {
    YAML::Node root;
    root["name"] = map.name;
    root["frame_id"] = map.header.frame_id;

    double common_size = -1.0;
    if (!map.markers.empty()) {
        common_size = map.markers[0].size;
        for (const auto& m : map.markers) {
            if (std::abs(static_cast<double>(m.size) - common_size) > 1e-9) {
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
        YAML::Node entry =
            YAML::convert<clover2_pose_msgs::msg::Marker>::encode(m);
        if (common_size > 0.0) {
            entry.remove("size");
        }
        markers.push_back(entry);
    }
    root["markers"] = markers;

    std::ofstream out(filename);
    if (!out.good()) {
        throw std::runtime_error(
            "Unable to open for write: " + filename.string() +
            ", reason: " + strerror(errno));
    }
    out << root;
}

void save_txt(const std::filesystem::path& filename,
              const clover2_pose_msgs::msg::MarkerMap& map) {
    std::ofstream out(filename);
    if (!out.good()) {
        throw std::runtime_error(
            "Unable to open for write: " + filename.string() +
            ", reason: " + strerror(errno));
    }

    out << std::setprecision(12);
    for (const auto& m : map.markers) {
        tf2::Quaternion q(m.pose.pose.orientation.x, m.pose.pose.orientation.y,
                          m.pose.pose.orientation.z, m.pose.pose.orientation.w);
        double roll = 0.0;
        double pitch = 0.0;
        double yaw = 0.0;
        tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);

        out << m.id << ' ' << m.size << ' ' << m.pose.pose.position.x << ' '
            << m.pose.pose.position.y << ' ' << m.pose.pose.position.z << ' '
            << yaw << ' ' << pitch << ' ' << roll << '\n';
    }
}

}  // namespace

fs_provider::fs_provider(const std::filesystem::path& filename,
                         rclcpp::Logger logger)
    : m_logger(logger)
    , m_filename(filename) {}

void fs_provider::load() {
    reset();

    const std::string ext = m_filename.extension().string();
    if (ext == ".txt") {
        RCLCPP_DEBUG(m_logger, "Map format by extension: txt (%s)",
                     m_filename.c_str());
        load_txt(m_logger, m_filename, m_map);
    } else if (ext == ".yaml" || ext == ".yml") {
        RCLCPP_DEBUG(m_logger, "Map format by extension: yaml (%s)",
                     m_filename.c_str());
        load_yaml(m_logger, m_filename, m_map);
    } else {
        throw std::runtime_error("Unknown map file extension '" + ext +
                                 "' (supported: .txt, .yaml, .yml)");
    }
}

void fs_provider::save() const {
    const std::string ext = m_filename.extension().string();
    if (ext == ".txt") {
        RCLCPP_DEBUG(m_logger, "Saving map as txt (%s)", m_filename.c_str());
        save_txt(m_filename, m_map);
    } else if (ext == ".yaml" || ext == ".yml") {
        RCLCPP_DEBUG(m_logger, "Saving map as yaml (%s)", m_filename.c_str());
        save_yaml(m_filename, m_map);
    } else {
        throw std::runtime_error("Unknown map file extension '" + ext +
                                 "' (supported: .txt, .yaml, .yml)");
    }
}

const clover2_pose_msgs::msg::MarkerMap& fs_provider::get_map() const {
    return m_map;
}

clover2_pose_msgs::msg::MarkerMap& fs_provider::get_map() {  //
    return m_map;
}

void fs_provider::reset() { m_map.markers.clear(); }

}  // namespace clover2::map::io
