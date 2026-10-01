#include <clover2/cam_feature/data/solve_pnp_method.hpp>

namespace clover2::cam_feature::data {

namespace {

const std::unordered_map<std::string, cv::SolvePnPMethod> str_to_method = {
    {"iterative", cv::SOLVEPNP_ITERATIVE},
    {"p3p", cv::SOLVEPNP_P3P},
    {"ap3p", cv::SOLVEPNP_AP3P},
    {"eppnp", cv::SOLVEPNP_EPNP},
    {"dls", cv::SOLVEPNP_DLS},
    {"upnp", cv::SOLVEPNP_UPNP},
    {"ippe", cv::SOLVEPNP_IPPE},
    {"ippe_square", cv::SOLVEPNP_IPPE_SQUARE},
};

const std::unordered_map<cv::SolvePnPMethod, std::string> method_to_str = {
    {cv::SOLVEPNP_ITERATIVE, "iterative"},
    {cv::SOLVEPNP_P3P, "p3p"},
    {cv::SOLVEPNP_AP3P, "ap3p"},
    {cv::SOLVEPNP_EPNP, "eppnp"},
    {cv::SOLVEPNP_DLS, "dls"},
    {cv::SOLVEPNP_UPNP, "upnp"},
    {cv::SOLVEPNP_IPPE, "ippe"},
    {cv::SOLVEPNP_IPPE_SQUARE, "ippe_square"},
};

}  // namespace

cv::SolvePnPMethod solve_pnp_method::from_str(const std::string& str) {
    auto it = str_to_method.find(str);
    if (it != str_to_method.end()) {
        return it->second;
    }

    throw std::invalid_argument("Invalid solve_pnp_method string.");
}

std::string solve_pnp_method::to_str(cv::SolvePnPMethod method) {
    auto it = method_to_str.find(method);
    if (it != method_to_str.end()) {
        return it->second;
    }

    throw std::invalid_argument("Invalid solve_pnp_method method.");
}

}  // namespace clover2::cam_feature::data
