
// clover2
#include <clover2/cam_feature/detail/maker_base.hpp>
#include <clover2_common/util/parameter.hpp>

// opencv
#include <opencv2/aruco.hpp>

// STL
#include <memory>
#include <stdexcept>
#include <string>

#define WATCH_PARAM(_param_name, _type)                                    \
    declare_and_watch_parameter(                                           \
        get_name() + "." #_param_name, m_detector_parameters->_param_name, \
        [&](const rclcpp::Parameter& p) {                                  \
            m_detector_parameters->_param_name = p.as_##_type();           \
        });

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

    void on_configure(
        const std::string& name,
        const std::shared_ptr<clover2_common::node_context>& node_context,
        const std::shared_ptr<clover2_map::client>& map_client)
        override final {
        _configure(name, node_context, map_client);
        m_map_client = map_client;

        auto parameters_interface =
            node_context->get_node_parameters_interface();

        clover2_common::util::declare_parameter_if_not_declared(
            parameters_interface, name + ".marker_dict", "4X4_50");

        m_detector_parameters = cv::aruco::DetectorParameters::create();
        declare_detector_params();

        rebuild_dictionary(resolve_dictionary_id());
    }

    void on_activate() override final { _activate(); }
    void on_deactivate() override final { _deactivate(); }

    void on_cleanup() override final {
        m_detector_parameters.reset();
        m_dictionary.reset();
        m_map_client.reset();
        m_dictionary_id.clear();

        _cleanup();
    }

protected:
    void detect_markers(
        const cv::Mat& image, std::vector<int>& ids,
        std::vector<std::vector<cv::Point2f>>& corners) override final {
        ensure_dictionary();

        std::vector<std::vector<cv::Point2f>> rejected;
        cv::aruco::detectMarkers(image, m_dictionary, corners, ids,
                                 m_detector_parameters, rejected);
    }

private:
    void declare_detector_params() {
        WATCH_PARAM(adaptiveThreshWinSizeMin, int);
        WATCH_PARAM(adaptiveThreshWinSizeMax, int);
        WATCH_PARAM(adaptiveThreshWinSizeStep, int);
        WATCH_PARAM(adaptiveThreshConstant, double);
        WATCH_PARAM(minMarkerPerimeterRate, double);
        WATCH_PARAM(maxMarkerPerimeterRate, double);
        WATCH_PARAM(polygonalApproxAccuracyRate, double);
        WATCH_PARAM(minCornerDistanceRate, double);
        WATCH_PARAM(minDistanceToBorder, int);
        WATCH_PARAM(minMarkerDistanceRate, double);
        WATCH_PARAM(cornerRefinementMethod, int);
        WATCH_PARAM(cornerRefinementWinSize, int);
        WATCH_PARAM(cornerRefinementMaxIterations, int);
        WATCH_PARAM(cornerRefinementMinAccuracy, double);
        WATCH_PARAM(markerBorderBits, int);
        WATCH_PARAM(perspectiveRemovePixelPerCell, int);
        WATCH_PARAM(perspectiveRemoveIgnoredMarginPerCell, double);
        WATCH_PARAM(maxErroneousBitsInBorderRate, double);
        WATCH_PARAM(minOtsuStdDev, double);
        WATCH_PARAM(errorCorrectionRate, double);
    }

    // Dictionary from the map wins over the marker_dict parameter;
    // empty map dictionary means the parameter is used.
    std::string resolve_dictionary_id() const {
        if (m_map_client && m_map_client->valid()) {
            const auto dictionary = m_map_client->get_dictionary();
            if (!dictionary.empty()) {
                return dictionary;
            }
        }

        auto parameters_interface =
            m_node_context->get_node_parameters_interface();

        rclcpp::Parameter dictionary_id_param;
        parameters_interface->get_parameter(get_name() + ".marker_dict",
                                            dictionary_id_param);

        return dictionary_id_param.as_string();
    }

    void rebuild_dictionary(const std::string& dictionary_id) {
        auto id = marker_dictionary_map.find(dictionary_id);
        if (id == marker_dictionary_map.end()) {
            throw std::runtime_error("Unknown dictionary: " + dictionary_id);
        }

        m_dictionary = cv::makePtr<cv::aruco::Dictionary>(
            cv::aruco::getPredefinedDictionary(id->second));
        m_dictionary_id = dictionary_id;

        RCLCPP_INFO(get_logger(), "Using dictionary '%s'",
                    dictionary_id.c_str());
    }

    // Recreate the dictionary when the map (and so its dictionary)
    // changes at runtime.
    void ensure_dictionary() {
        const std::string dictionary_id = resolve_dictionary_id();
        if (dictionary_id == m_dictionary_id) {
            return;
        }

        try {
            rebuild_dictionary(dictionary_id);
        } catch (const std::exception& e) {
            RCLCPP_ERROR(get_logger(), "%s", e.what());
        }
    }

    cv::Ptr<cv::aruco::Dictionary> m_dictionary;
    cv::Ptr<cv::aruco::DetectorParameters> m_detector_parameters;
    std::shared_ptr<clover2_map::client> m_map_client;
    std::string m_dictionary_id;
};

}  // namespace clover2::cam_feature::plugins

#include <pluginlib/class_list_macros.hpp>

CAM_FEATURE_PLUGIN_REGISTER(clover2::cam_feature::plugins::aruco)
