#include <clover2_camera_utils/data/board/loader.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace clover2_camera_utils::data::board {
namespace {

using json = nlohmann::json;

bool is_supported_dictionary(const std::string& dictionary) {
    static constexpr std::array<std::string_view, 21> k_supported_dictionaries{
        "DICT_4X4_50",        "DICT_4X4_100",        "DICT_4X4_250",
        "DICT_4X4_1000",      "DICT_5X5_50",         "DICT_5X5_100",
        "DICT_5X5_250",       "DICT_5X5_1000",       "DICT_6X6_50",
        "DICT_6X6_100",       "DICT_6X6_250",        "DICT_6X6_1000",
        "DICT_7X7_50",        "DICT_7X7_100",        "DICT_7X7_250",
        "DICT_7X7_1000",      "DICT_ARUCO_ORIGINAL", "DICT_APRILTAG_16h5",
        "DICT_APRILTAG_25h9", "DICT_APRILTAG_36h10", "DICT_APRILTAG_36h11",
    };
    return std::ranges::find(k_supported_dictionaries, dictionary) !=
           k_supported_dictionaries.end();
}

chessboard_config parse_chessboard(const json& value) {
    const chessboard_config config{
        .rows = value.at("rows").get<uint32_t>(),
        .columns = value.at("columns").get<uint32_t>(),
        .square_size_m = value.at("square_size_m").get<double>(),
    };
    if (config.rows < 2 || config.columns < 2 || config.square_size_m <= 0.0) {
        throw std::invalid_argument(
            "chessboard rows and columns must be at least 2, and "
            "square_size_m must be positive");
    }
    return config;
}

charuco_config parse_charuco(const json& value) {
    const charuco_config config{
        .squares_rows = value.at("squares_rows").get<uint32_t>(),
        .squares_columns = value.at("squares_columns").get<uint32_t>(),
        .square_size_m = value.at("square_size_m").get<double>(),
        .marker_size_m = value.at("marker_size_m").get<double>(),
        .dictionary = value.at("dictionary").get<std::string>(),
    };
    if (config.squares_rows < 3 || config.squares_columns < 3 ||
        config.square_size_m <= 0.0 || config.marker_size_m <= 0.0 ||
        config.marker_size_m >= config.square_size_m) {
        throw std::invalid_argument(
            "charuco squares_rows and squares_columns must be at least 3, "
            "and marker_size_m must be positive and smaller than "
            "square_size_m");
    }
    if (!is_supported_dictionary(config.dictionary)) {
        throw std::invalid_argument("unsupported ArUco dictionary '" +
                                    config.dictionary + "'");
    }
    return config;
}

}  // namespace

config load_config(const std::filesystem::path& path) {
    if (path.empty() || !path.is_absolute()) {
        throw std::invalid_argument(
            "board configuration path must be absolute");
    }

    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("could not open board configuration '" +
                                 path.string() + "'");
    }

    const json value = json::parse(input);
    const auto type = value.at("type").get<std::string>();
    if (type == "chessboard") {
        return parse_chessboard(value);
    }
    if (type == "charuco") {
        return parse_charuco(value);
    }
    throw std::invalid_argument("unsupported board type '" + type + "'");
}

std::string_view type_id(const config& value) noexcept {
    return std::visit(
        [](const auto& board) -> std::string_view {
            using board_type = std::decay_t<decltype(board)>;
            if constexpr (std::is_same_v<board_type, chessboard_config>) {
                return "chessboard";
            } else {
                return "charuco";
            }
        },
        value);
}

}  // namespace clover2_camera_utils::data::board
