#pragma once

// clover2
#include <clover2_map/io/providers/base_provider.hpp>

// STL
#include <filesystem>

namespace clover2_map::io::providers {

class yaml : public base_provider {
public:
    yaml(const std::filesystem::path& filename, rclcpp::Logger logger);
    ~yaml() override = default;

    void load(clover2_map::map& map) override;
    void save(const clover2_map::map& map) const override;

private:
    void load_impl(clover2_map::map& map);
};

}  // namespace clover2_map::io::providers
