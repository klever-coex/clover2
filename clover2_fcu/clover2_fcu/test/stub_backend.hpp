#pragma once

// clover2
#include <clover2_fcu/backend/backend_base.hpp>
#include <clover2_fcu/capability/commands_interface.hpp>
#include <clover2_fcu/capability/setpoint_position_interface.hpp>
#include <clover2_fcu/capability/system_state_interface.hpp>
#include <clover2_fcu/context.hpp>

// third party
#include <Eigen/Geometry>

// STL
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace clover2_fcu::stub {

/// Test-only backend carrier for the bridge/mode_base lease tests. It is
/// a real pluginlib plugin ("stub") compiled as a test-support library;
/// test_lease installs it into a temporary ament prefix at runtime
/// (pluginlib discovers backends only through installed package XMLs).

/// Telemetry carrier: injections reach every live instance — the
/// client's provider and the bridge's mirror are separate objects.
class system_state : public capability::system_state_interface {
public:
    explicit system_state(const context& ctx)
        : capability::system_state_interface(ctx.clock) {}

    ~system_state() override;

    static void publish(const data::system_state& state);

    void inject(const data::system_state& state) { update_data(state); }

private:
    friend class factory;

    static void register_provider(const std::shared_ptr<system_state>& p);

    static std::mutex& providers_mutex();
    static std::vector<std::weak_ptr<system_state>>& providers();
};

class commands : public capability::commands_interface {
public:
    explicit commands(const context& ctx) { (void)ctx; }

    void arm(result_callback cb, bool run_preflight_checks) override;
    void disarm(result_callback cb) override;
    void set_mode(result_callback cb, data::flight_mode mode) override;
    void takeoff(result_callback cb, std::optional<double> altitude,
                 std::optional<double> heading) override;
    void land(result_callback cb) override;
    void rtl(result_callback cb) override;

    std::size_t pending_count(const std::string& name) const;
    void resolve_pending(const std::string& name, bool ok);

private:
    void issue(std::string name, result_callback cb);

    mutable std::mutex m_mutex;
    std::map<std::string, std::vector<result_callback>> m_pending;
};

/// Position setpoint sink recording update() calls.
class position_setpoint : public capability::setpoint_position_interface {
public:
    struct update_record {
        Eigen::Vector3d position{Eigen::Vector3d::Zero()};
        std::optional<double> heading{};
    };

    explicit position_setpoint(const context& ctx) { (void)ctx; }

    void update(const Eigen::Vector3d& position,
                std::optional<double> heading) override;

    std::vector<update_record> updates() const;

private:
    mutable std::mutex m_mutex;
    std::vector<update_record> m_updates;
};

class factory : public backend::backend_base {
public:
    factory();

    std::string protocol() const override;

    void do_initialize() override;
};

}  // namespace clover2_fcu::stub
