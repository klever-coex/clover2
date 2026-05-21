
// clover2
#include <clover2/cam_feature/detail/maker_base.hpp>
#include <clover2_common/util/parameter.hpp>

// opencv
#include <opencv2/aruco.hpp>

// STL
#include <stdexcept>
#include <string>

#define READ_PARAM(_node, _name, _struct, _param_name) \
    clover2_common::util::safe_declare_and_get(       \
        _node, _name + "." #_param_name, _struct->_param_name)

namespace {

const static std::unordered_map<std::string, int> marker_dictionary_map = {
    {"4X4_50", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50},
    {"4X4_100", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_100},
    {"4X4_250", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_250},
    {"4X4_1000", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_1000},
    {"5X5_50", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_5X5_50},
    {"5X5_100", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_5X5_100},
    {"5X5_250", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_5X5_250},
    {"5X5_1000", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_5X5_1000},
    {"6X6_50", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_6X6_50},
    {"6X6_100", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_6X6_100},
    {"6X6_250", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_6X6_250},
    {"6X6_1000", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_6X6_1000},
    {"7X7_50", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_7X7_50},
    {"7X7_100", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_7X7_100},
    {"7X7_250", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_7X7_250},
    {"7X7_1000", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_7X7_1000},
    {"ARUCO_ORIGINAL",
     cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_ARUCO_ORIGINAL},
    {"APRILTAG_16h5",
     cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_APRILTAG_16h5},
    {"APRILTAG_25h9",
     cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_APRILTAG_25h9},
    {"APRILTAG_36h10",
     cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_APRILTAG_36h10},
    {"APRILTAG_36h11",
     cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_APRILTAG_36h11},
};

}

namespace clover2::cam_feature::plugins {

class aruco final : public clover2::cam_feature::detail::maker_base {
public:
    explicit aruco() = default;
    virtual ~aruco() = default;

    void on_configure(const std::string& name,
                      const rclcpp_lifecycle::LifecycleNode::WeakPtr& node,
                      const std::shared_ptr<clover2::map::client>& map_client)
        override final {
        _configure(name, node, map_client);

        auto node_ptr = node.lock();

        std::string dictionary_id = "4X4_50";  // default value
        clover2_common::util::safe_declare_and_get(
            node_ptr, name + ".marker_dict", dictionary_id);

        auto id = marker_dictionary_map.find(dictionary_id);
        if (id == marker_dictionary_map.end()) {
            throw std::runtime_error("Unknown dictionary: " + dictionary_id);
        }

        m_dictionary = cv::makePtr<cv::aruco::Dictionary>(
            cv::aruco::getPredefinedDictionary(id->second));

        m_detector_parameters = declare_detector_params(node_ptr, name);
        if (!m_detector_parameters) {
            throw std::runtime_error("Fail to configure detector parameters");
        }
    }

    void on_activate() override final { _activate(); }
    void on_deactivate() override final { _deactivate(); }

    void on_cleanup() override final {
        m_detector_parameters.reset();
        m_dictionary.reset();

        _cleanup();
    }

protected:
    void detect_markers(
        const cv::Mat& image, std::vector<int>& ids,
        std::vector<std::vector<cv::Point2f>>& corners) override final {
        std::vector<std::vector<cv::Point2f>> rejected;
        cv::aruco::detectMarkers(image, m_dictionary, corners, ids,
                                 m_detector_parameters, rejected);
    }

private:
    cv::Ptr<cv::aruco::DetectorParameters> declare_detector_params(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
        const std::string& name) {
        auto p = cv::aruco::DetectorParameters::create();

        READ_PARAM(node, name, p, adaptiveThreshWinSizeMin);
        READ_PARAM(node, name, p, adaptiveThreshWinSizeMax);
        READ_PARAM(node, name, p, adaptiveThreshWinSizeStep);
        READ_PARAM(node, name, p, adaptiveThreshConstant);
        READ_PARAM(node, name, p, minMarkerPerimeterRate);
        READ_PARAM(node, name, p, maxMarkerPerimeterRate);
        READ_PARAM(node, name, p, polygonalApproxAccuracyRate);
        READ_PARAM(node, name, p, minCornerDistanceRate);
        READ_PARAM(node, name, p, minDistanceToBorder);
        READ_PARAM(node, name, p, minMarkerDistanceRate);
        READ_PARAM(node, name, p, cornerRefinementMethod);
        READ_PARAM(node, name, p, cornerRefinementWinSize);
        READ_PARAM(node, name, p, polygonalApproxAccuracyRate);
        READ_PARAM(node, name, p, cornerRefinementMinAccuracy);
        READ_PARAM(node, name, p, markerBorderBits);
        READ_PARAM(node, name, p, perspectiveRemovePixelPerCell);
        READ_PARAM(node, name, p, perspectiveRemoveIgnoredMarginPerCell);
        READ_PARAM(node, name, p, maxErroneousBitsInBorderRate);
        READ_PARAM(node, name, p, minOtsuStdDev);
        READ_PARAM(node, name, p, errorCorrectionRate);

        return p;
    }

    cv::Ptr<cv::aruco::Dictionary> m_dictionary;
    cv::Ptr<cv::aruco::DetectorParameters> m_detector_parameters;
};

}  // namespace clover2::cam_feature::plugins

#include <pluginlib/class_list_macros.hpp>

CAM_FEATURE_PLUGIN_REGISTER(clover2::cam_feature::plugins::aruco)
