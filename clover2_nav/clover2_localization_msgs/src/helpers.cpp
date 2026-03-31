#include <clover2_localization_msgs/helpers.hpp>

#include <stdexcept>
#include <string>

namespace clover2_localization_msgs::helpers {

feature_id feature_info::id() const {
    feature_id id = 0;

    switch (type) {
        case feature_type::aruco:
            id = marker_id.value();
            id = (static_cast<feature_id>(type) << 24);
            break;
        default:
            throw std::runtime_error("Invalid feature type " +
                                     std::to_string(type));
            break;
    }

    return id;
}

void fromMsg(const clover2_localization_msgs::msg::FeatureInfo& in, feature_info& out) {

    switch (in.type) {
        case clover2_localization_msgs::msg::FeatureInfo::TYPE_ARUCO:
            out.marker_id = in.maker_id;
            out.type = feature_type::aruco;
            break;
        default:
            throw std::runtime_error("Invalid feature msg type " +
                                     std::to_string(in.type));
            break;
    }
}

void toMsg(const feature_info& in, clover2_localization_msgs::msg::FeatureInfo& out) {
    switch (in.type) {
        case feature_type::aruco:
            out.type = clover2_localization_msgs::msg::FeatureInfo::TYPE_ARUCO;
            out.maker_id = in.marker_id.value();
            break;
        default:
            throw std::runtime_error("Invalid feature type " +
                                     std::to_string(in.type));
            break;
    }
}

}  // namespace clover2_localization_msgs::helpers
