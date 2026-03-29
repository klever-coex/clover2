
#include <clover2_cam_feature/detail/maker_base.hpp>
#include <opencv2/aruco.hpp>

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

namespace clover2_cam_feature::plugins {

class aruco : public clover2_cam_feature::detail::maker_base {
public:
    aruco() = default;

    ~aruco() override = default;

protected:
    void init([[maybe_unused]] const clover2_cam_feature::plugin_context& ctx)
        override {
        m_detector_parameters = cv::aruco::DetectorParameters::create();

        declare_and_watch_parameter<std::string>(
            "marker_dict", "4X4_250",
            [this](const rclcpp::Parameter& p) {
                auto dictionary_id = marker_dictionary_map.find(p.as_string());
                if (dictionary_id == marker_dictionary_map.end()) {
                    throw std::runtime_error("invalid marker type " +
                                             p.as_string());
                }

                m_dictionary = cv::makePtr<cv::aruco::Dictionary>(
                    cv::aruco::getPredefinedDictionary(dictionary_id->second));
            },
            "Used marker dictionary");
    }

    void detect_markers(
        const cv::Mat& image, std::vector<int>& ids,
        std::vector<std::vector<cv::Point2f>>& corners) override {
        std::vector<std::vector<cv::Point2f>> rejected;
        cv::aruco::detectMarkers(image, m_dictionary, corners, ids,
                                 m_detector_parameters, rejected);
    }

private:
    cv::Ptr<cv::aruco::Dictionary> m_dictionary;
    cv::Ptr<cv::aruco::DetectorParameters> m_detector_parameters;
};

}  // namespace clover2_cam_feature::plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(clover2_cam_feature::plugins::aruco,
                       clover2_cam_feature::base_plugin)
