#include <clover2_aruco/utils/map_io.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/logging.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <yaml-cpp/yaml.h>

#include <fstream>

namespace YAML {
template <>
struct convert<clover2_aruco_msgs::msg::Marker> {
    static bool decode(const Node& node,
                       clover2_aruco_msgs::msg::Marker& marker) {
        if (!node.IsMap()) {
            return false;
        }

        if (!node["id"] || !node["size"]) {
            return false;
        }

        marker.id = node["id"].as<int>();
        marker.size = node["size"].as<double>();

        marker.pose.position.x = 0.0;
        marker.pose.position.y = 0.0;
        marker.pose.position.z = 0.0;

        if (node["pose"]) {
            auto& pose = node["pose"];
            if (pose["x"]) {
                marker.pose.position.x = pose["x"].as<double>();
            }

            if (pose["y"]) {
                marker.pose.position.y = pose["y"].as<double>();
            }

            if (pose["z"]) {
                marker.pose.position.z = pose["z"].as<double>();
            }
        }

        tf2::Quaternion q;
        if (node["rot"]) {
            auto& rot = node["rot"];
            double roll = 0.0, pitch = 0.0, yaw = 0.0;

            if (rot["roll"]) {
                roll = rot["roll"].as<double>();
            }

            if (rot["pitch"]) {
                pitch = rot["pitch"].as<double>();
            }

            if (rot["yaw"]) {
                yaw = rot["yaw"].as<double>();
            }

            q.setRPY(roll, pitch, yaw);
        } else if (node["quat"]) {
            auto& quat = node["quat"];

            if (!quat["x"] || !quat["y"] || !quat["z"] || !quat["w"]) {
                return false;
            }

            q.setValue(quat["x"].as<double>(), quat["y"].as<double>(),
                       quat["z"].as<double>(), quat["w"].as<double>());
        } else {
            q.setRPY(0.0, 0.0, 0.0);
        }

        marker.pose.orientation.x = q.x();
        marker.pose.orientation.y = q.y();
        marker.pose.orientation.z = q.z();
        marker.pose.orientation.w = q.w();

        if (node["frame_id"]) {
            marker.marker_frame_id = node["frame_id"].as<std::string>();
        } else {
            marker.marker_frame_id = "aruco_" + std::to_string(marker.id);
        }

        return true;
    }
};

}  // namespace YAML

namespace clover2_aruco::utils {

void load_from_yaml(const std::filesystem::path& filename,
                    clover2_aruco_msgs::msg::MarkerMap& map) {
    auto logger = rclcpp::get_logger("load_from_yaml");

    YAML::Node config = YAML::LoadFile(filename);

    if (config["name"]) {
        map.name = config["name"].as<std::string>();
    } else {
        map.name = filename.stem();
    }

    RCLCPP_DEBUG(logger, "Parsing map '%s' form '%s'", map.name.c_str(),
                 filename.c_str());

    if (config["frame_id"]) {
        map.header.frame_id = config["frame_id"].as<std::string>();
    } else {
        RCLCPP_WARN(logger, "Map frame_id not provided. Using default");
        map.header.frame_id = "map";
    }

    if (config["markers"].IsSequence()) {
        for (const auto& it : config["markers"]) {
            auto marker = it.as<clover2_aruco_msgs::msg::Marker>();
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

        if (!(s >> marker.id >> marker.size >> marker.pose.position.x >>
              marker.pose.position.y)) {
            RCLCPP_ERROR(
                logger,
                "Not enough data in line: %s; "
                "Each marker must have at least id, length, x, y fields",
                line.c_str());
            continue;
        }

        if (!(s >> marker.pose.position.z)) {
            RCLCPP_DEBUG(logger,
                         "No z coordinate provided for marker %d, assuming 0",
                         marker.id);
            marker.pose.position.z = 0;
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

        tf2::Quaternion q;
        q.setRPY(roll, pitch, yaw);

        marker.pose.orientation.x = q.x();
        marker.pose.orientation.y = q.y();
        marker.pose.orientation.z = q.z();
        marker.pose.orientation.w = q.w();

        map.markers.push_back(marker);
    }

    map.header.frame_id = "map";
}

}  // namespace clover2_aruco::utils
