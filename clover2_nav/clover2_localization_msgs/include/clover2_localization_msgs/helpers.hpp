#pragma once

#include <clover2_localization_msgs/msg/feature_info.hpp>

#include <optional>

namespace clover2_localization_msgs::helpers {

enum class feature_type : uint8_t { aruco };

using feature_id = int;

struct feature_info {
    feature_type type;
    std::optional<int> marker_id;

    feature_id id() const;
};

void fromMsg(const clover2_localization_msgs::msg::FeatureInfo& in,
             feature_info& out);
void toMsg(const feature_info& in,
           clover2_localization_msgs::msg::FeatureInfo& out);

}  // namespace clover2_localization_msgs::helpers
