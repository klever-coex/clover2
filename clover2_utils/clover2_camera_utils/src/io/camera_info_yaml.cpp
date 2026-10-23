#include <clover2_camera_utils/io/camera_info_yaml.hpp>

#include <fstream>
#include <iomanip>
#include <system_error>

namespace clover2_camera_utils::io {

bool save_camera_info_yaml(const sensor_msgs::msg::CameraInfo& camera_info,
                           const std::filesystem::path& path,
                           std::string& error) {
    std::error_code filesystem_error;
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path(), filesystem_error);
        if (filesystem_error) {
            error = "cannot create directory '" + path.parent_path().string() +
                    "': " + filesystem_error.message();
            return false;
        }
    }

    std::ofstream output(path);
    if (!output) {
        error = "cannot open '" + path.string() + "' for writing";
        return false;
    }

    const auto write_matrix = [&output](const char* name, const auto& values,
                                        size_t rows, size_t columns) {
        output << name << ":\n"
               << "  rows: " << rows << "\n"
               << "  cols: " << columns << "\n"
               << "  data: [";
        for (size_t index = 0; index < values.size(); ++index) {
            if (index != 0) {
                output << ", ";
            }
            output << values[index];
        }
        output << "]\n";
    };

    output << std::setprecision(17);
    output << "image_width: " << camera_info.width << "\n"
           << "image_height: " << camera_info.height << "\n"
           << "camera_name: camera_calibration\n";
    write_matrix("camera_matrix", camera_info.k, 3, 3);
    output << "distortion_model: " << camera_info.distortion_model << "\n";
    write_matrix("distortion_coefficients", camera_info.d, 1,
                 camera_info.d.size());
    write_matrix("rectification_matrix", camera_info.r, 3, 3);
    write_matrix("projection_matrix", camera_info.p, 3, 4);
    output << "binning_x: " << camera_info.binning_x << "\n"
           << "binning_y: " << camera_info.binning_y << "\n"
           << "roi:\n"
           << "  x_offset: " << camera_info.roi.x_offset << "\n"
           << "  y_offset: " << camera_info.roi.y_offset << "\n"
           << "  height: " << camera_info.roi.height << "\n"
           << "  width: " << camera_info.roi.width << "\n"
           << "  do_rectify: "
           << (camera_info.roi.do_rectify ? "true" : "false") << "\n";
    if (!output) {
        error = "failed while writing '" + path.string() + "'";
        return false;
    }
    return true;
}

}  // namespace clover2_camera_utils::io