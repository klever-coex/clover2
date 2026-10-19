/**
 * @file factory.hpp
 * @brief Provides the fake backend factory and its test-facing registry.
 */

#pragma once

// clover2
#include <clover2_fcu/backend/backend_base.hpp>
#include <clover2_fcu/capability/estimator_interface.hpp>
#include <clover2_fcu/capability/parameters_interface.hpp>
#include <clover2_fcu/data/battery.hpp>
#include <clover2_fcu/data/odometry.hpp>
#include <clover2_fcu/data/system_state.hpp>

// third party
#include <Eigen/Geometry>

// STL
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace clover2_fcu::fake {

class fake_system_state;
class fake_odometry;
class fake_battery;
class fake_estimator;

/**
 * @brief Test-facing registry of live fake providers.
 *
 * The fake backend keeps no transport: tests inject telemetry snapshots
 * and observe commands/setpoints through this singleton. Live telemetry
 * providers are tracked with weak pointers; entries expire together with
 * the owning client. It is shared by all fake providers across all
 * clients in the process.
 */
class registry {
public:
    /// @brief A recorded setpoint update with the gate state at the call.
    template <typename payload_t>
    struct recorded_update {
        payload_t payload;
        bool was_streaming{false};
    };

    struct position_payload {
        Eigen::Vector3d position{Eigen::Vector3d::Zero()};
        std::optional<double> heading{};
    };

    struct velocity_payload {
        Eigen::Vector3d velocity{Eigen::Vector3d::Zero()};
        std::optional<double> yaw_rate{};
    };

    struct attitude_payload {
        Eigen::Quaterniond attitude{Eigen::Quaterniond::Identity()};
        double thrust{0.0};
    };

    struct rates_payload {
        Eigen::Vector3d body_rates{Eigen::Vector3d::Zero()};
        double thrust{0.0};
    };

    static registry& instance();

    /// @brief Reset all logs, results and parameters between tests.
    void reset();

    // Telemetry injection (reaches every live fake telemetry provider).

    void publish_system_state(const data::system_state& state);
    void publish_odometry(const data::odometry& odom);
    void publish_battery(const data::battery& battery);
    void publish_estimator_status(const data::estimator_status& status);

    // Commands behavior and observation.

    /// @brief Sticky result returned by subsequent fake command calls.
    void set_command_result(result value);

    /// @brief Command names in call order ("arm", "land", ...).
    const std::vector<std::string> command_log() const;

    // Parameters storage (shared by the fake parameters provider).

    std::map<std::string, capability::parameter_value> parameters;

    // Estimator behavior and observation.

    /// @brief A recorded odometry publish into the estimator.
    struct recorded_odometry {
        data::odometry odometry;
        capability::estimator_source source{
            capability::estimator_source::vision};
    };

    /// @brief Sticky result returned by subsequent fake reset calls.
    void set_reset_result(result value);

    /// @brief Reset scopes in call order.
    const std::vector<capability::estimator_reset> reset_log() const;

    /// @brief Origin set by the last set_origin() (nullopt until then).
    std::optional<data::geo_point> estimator_origin;

    /// @brief Odometry publishes in call order.
    std::vector<recorded_odometry> odometry_updates;

    // Setpoint observation.

    std::vector<recorded_update<position_payload>> position_updates;
    std::vector<recorded_update<velocity_payload>> velocity_updates;
    std::vector<recorded_update<attitude_payload>> attitude_updates;
    std::vector<recorded_update<rates_payload>> rates_updates;

private:
    friend class factory;
    friend class fake_commands;
    friend class fake_parameters;
    friend class fake_estimator;
    friend class fake_position_setpoint;
    friend class fake_velocity_setpoint;
    friend class fake_attitude_setpoint;
    friend class fake_rates_setpoint;

    registry() = default;

    void register_system_state(const std::shared_ptr<fake_system_state>& iface);
    void register_odometry(const std::shared_ptr<fake_odometry>& iface);
    void register_battery(const std::shared_ptr<fake_battery>& iface);
    void register_estimator(const std::shared_ptr<fake_estimator>& iface);

    result log_command(std::string name);
    result log_reset(capability::estimator_reset scope);
    result record_origin(const data::geo_point& origin);

    mutable std::mutex m_mutex;
    std::vector<std::weak_ptr<fake_system_state>> m_system_states;
    std::vector<std::weak_ptr<fake_odometry>> m_odometries;
    std::vector<std::weak_ptr<fake_battery>> m_batteries;
    std::vector<std::weak_ptr<fake_estimator>> m_estimators;
    std::vector<std::string> m_command_log;
    result m_command_result{result::value::success};
    std::vector<capability::estimator_reset> m_reset_log;
    result m_reset_result{result::value::success};
};

/**
 * @brief In-memory backend factory ("fake" protocol).
 *
 * Serves every MVP capability. Builders inject the dependencies
 * (base_interface, the shared test registry) into their providers;
 * setpoint providers are created per mode via create_setpoint(). Useful
 * for tests and for developing against a simulated flight stack without
 * any transport.
 */
class factory : public clover2_fcu::backend::backend_base {
public:
    factory();

    std::string protocol() const override;

    void do_initialize(
        std::shared_ptr<capability::base_interface> base) override;
};

}  // namespace clover2_fcu::fake
