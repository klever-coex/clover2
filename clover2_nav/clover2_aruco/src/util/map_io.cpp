// clover2
#include <clover2/aruco/util/map_io.hpp>

// ROS2
#include <rclcpp/logger.hpp>
#include <rclcpp/logging.hpp>
#include <tf2/LinearMath/Quaternion.hpp>

// yaml
#include <yaml-cpp/yaml.h>

// STL
#include <fstream>

namespace YAML {
template <>
struct convert<clover2_aruco_msgs::msg::Marker> {
    static bool decode(const Node& node,
                       clover2_aruco_msgs::msg::Marker& marker) {
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
};

}  // namespace YAML

namespace clover2::aruco::util {

void load_from_yaml(const std::filesystem::path& filename,
                    clover2_aruco_msgs::msg::MarkerMap& map) {
    auto logger = rclcpp::get_logger("load_from_yaml");

    YAML::Node config = YAML::LoadFile(filename);
    map.name = config["name"].as<std::string>(filename.stem());

    RCLCPP_DEBUG(logger, "Parsing map '%s' form '%s'", map.name.c_str(),
                 filename.c_str());

    double default_size = config["default_size"].as<double>(-1.0);
    map.header.frame_id = config["frame_id"].as<std::string>("map");

    if (config["markers"].IsSequence()) {
        for (const auto& it : config["markers"]) {
            auto marker = it.as<clover2_aruco_msgs::msg::Marker>();

            if (marker.size < 0.0 && default_size < 0.0) {
                RCLCPP_ERROR(logger,
                             "Missing size property for %d marker. Default "
                             "size also dont exist.",
                             marker.id);
                throw std::runtime_error("Missing marker size");
            }

            // override marker size if needed
            if (marker.size < 0.0) {
                marker.size = default_size;
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

void load_from_txt(const std::filesystem::path& filename,
                   clover2_aruco_msgs::msg::MarkerMap& map) {
    auto logger = rclcpp::get_logger("load_from_txt");
    map.name = filename.stem();
    map.header.frame_id = map.name;

    std::ifstream f(filename);
    if (!f.good()) {
        throw std::runtime_error("Unable to open " + filename.string() +
                                 ", reason: " + strerror(errno));
    }

    std::string line;
    while (std::getline(f, line)) {
        clover2_aruco_msgs::msg::Marker marker;
        double yaw, pitch, roll;

        std::istringstream s(line);

        char first = 0;
        if (!(s >> first) || first == '#') {
            continue;
        }

        if (std::isdigit(first)) {
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
            marker.pose.pose.position.z = 0;
        }

        if (!(s >> yaw)) {
            RCLCPP_DEBUG(logger, "No yaw provided for marker %d, assuming 0",
                         marker.id);
            yaw = 0;
        }

        if (!(s >> pitch)) {
            RCLCPP_DEBUG(logger, "No pitch provided for marker %d, assuming 0",
                         marker.id);
            pitch = 0;
        }

        if (!(s >> roll)) {
            RCLCPP_DEBUG(logger, "No roll provided for marker %d, assuming 0",
                         marker.id);
            roll = 0;
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

}  // namespace clover2::aruco::util
