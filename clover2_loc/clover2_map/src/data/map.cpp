#include <clover2_map/data/map.hpp>

// STL
#include <utility>

namespace clover2_map {

void map::add_marker(marker&& m) {
    markers[m.id] = std::move(m);
}

void map::add_marker(const marker& m) {
    markers[m.id] = m;
}

void map::to_msg(clover2_pose_msgs::msg::MarkerMap& msg) const {
    msg.header.frame_id = frame_id;
    msg.name = name;
    msg.dictionary = dictionary;
    msg.markers.clear();

    for (const auto& [_, m] : markers) {
        clover2_pose_msgs::msg::Marker out;
        m.to_msg(out);
        msg.markers.push_back(std::move(out));
    }
}

map map::from_msg(const clover2_pose_msgs::msg::MarkerMap& msg) {
    map m;
    m.name = msg.name;
    m.frame_id = msg.header.frame_id;
    m.dictionary = msg.dictionary;

    m.markers.reserve(msg.markers.size());
    for (const auto& it : msg.markers) {
        m.markers[it.id] = marker::from_msg(it);
    }

    return m;
}

}  // namespace clover2_map
