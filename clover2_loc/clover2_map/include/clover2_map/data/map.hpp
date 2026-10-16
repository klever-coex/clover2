#pragma once

// clover2
#include <clover2_map/data/marker.hpp>
#include <clover2_pose_msgs/msg/marker_map.hpp>

// STL
#include <string>
#include <unordered_map>

namespace clover2_map {

class map {
public:
    std::string name;
    std::string frame_id = "map";
    int version = 0;
    std::string dictionary;
    std::unordered_map<int, marker> markers;

    void add_marker(marker&& m);
    void add_marker(const marker& m);

    void to_msg(clover2_pose_msgs::msg::MarkerMap& msg) const;
    static map from_msg(const clover2_pose_msgs::msg::MarkerMap& msg);
};

}  // namespace clover2_map
