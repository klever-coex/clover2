#include <Eigen/Dense>

#include <iostream>
#include <memory>
#include <vector>

namespace clover2_aruco::utils {

class ekf {
private:
    // State: [x, y, z, qw, qx, qy, qz, vx, vy, vz, wx, wy, wz] (13 dimensions)
    // Position (3), Quaternion (4), Linear velocity (3), Angular velocity (3)
    static constexpr int STATE_DIM = 13;
    static constexpr int POS_DIM = 3;
    static constexpr int QUAT_DIM = 4;
    static constexpr int VEL_DIM = 3;
    static constexpr int ANG_VEL_DIM = 3;

    // Measurement: [marker_id, x, y, z, qw, qx, qy, qz] from each camera
    static constexpr int MEAS_DIM = 8;  // ID + pose (7)

    Eigen::VectorXd m_x;  // State vector
    Eigen::MatrixXd m_P;  // Covariance matrix
    double dt_;           // Time step

    // Process noise covariance
    Eigen::MatrixXd m_Q;
    // Measurement noise covariance
    Eigen::MatrixXd m_R;

public:
    ekf(double dt = 0.1)
        : dt_(dt) {
        // Initialize state (start at origin, identity orientation, zero
        // velocity)
        m_x = Eigen::VectorXd::Zero(STATE_DIM);
        m_x(3) = 1.0;  // qw = 1 (identity quaternion)

        // Initialize covariance
        m_P = Eigen::MatrixXd::Identity(STATE_DIM, STATE_DIM) * 0.1;

        // Initialize process noise (tune these based on your system)
        m_Q = Eigen::MatrixXd::Identity(STATE_DIM, STATE_DIM) * 0.01;

        // Initialize measurement noise (tune based on camera accuracy)
        m_R = Eigen::MatrixXd::Identity(MEAS_DIM - 1, MEAS_DIM - 1) *
              0.1;  // Exclude ID
    }

    // Set initial state
    void initialize(const Eigen::Vector3d& position,
                    const Eigen::Quaterniond& orientation) {
        m_x.segment<POS_DIM>(0) = position;
        m_x.segment<QUAT_DIM>(3) = Eigen::Vector4d(
            orientation.w(), orientation.x(), orientation.y(), orientation.z());
        m_x.segment<VEL_DIM>(7) = Eigen::Vector3d::Zero();
        m_x.segment<ANG_VEL_DIM>(10) = Eigen::Vector3d::Zero();
    }

    // Prediction step (constant velocity model)
    void predict() {
        // State transition function f(x)
        Eigen::VectorXd x_pred = stateTransition(m_x);

        // State transition Jacobian F = df/dx
        Eigen::MatrixXd F = stateTransitionJacobian(m_x);

        // Predict covariance: P = F * P * F^T + Q
        m_P = F * m_P * F.transpose() + m_Q;
        m_x = x_pred;
    }

    // Update step with multiple camera measurements
    void update(const std::vector<Eigen::VectorXd>& measurements) {
        if (measurements.empty()) return;

        // For simplicity, we'll process measurements sequentially
        // In a more advanced version, you could use batch update
        for (const auto& z : measurements) {
            updateSingleMeasurement(z);
        }
    }

    // Get current state
    Eigen::Vector3d getPosition() const { return m_x.segment<POS_DIM>(0); }
    Eigen::Quaterniond getOrientation() const {
        auto q_vec = m_x.segment<QUAT_DIM>(3);
        return Eigen::Quaterniond(q_vec(0), q_vec(1), q_vec(2), q_vec(3))
            .normalized();
    }
    Eigen::Vector3d getLinearVelocity() const {
        return m_x.segment<VEL_DIM>(7);
    }
    Eigen::Vector3d getAngularVelocity() const {
        return m_x.segment<ANG_VEL_DIM>(10);
    }

private:
    // State transition function (constant velocity model with quaternion
    // propagation)
    Eigen::VectorXd stateTransition(const Eigen::VectorXd& x) {
        Eigen::VectorXd x_next = x;

        // Position update: p = p + v * dt
        x_next.segment<POS_DIM>(0) += x.segment<VEL_DIM>(7) * dt_;

        // Orientation update: q = q ⊗ exp(ω * dt/2)
        Eigen::Vector3d ang_vel = x.segment<ANG_VEL_DIM>(10);
        Eigen::Quaterniond delta_q = angleAxisToQuaternion(ang_vel * dt_);
        Eigen::Quaterniond current_q(x(3), x(4), x(5), x(6));
        Eigen::Quaterniond new_q = current_q * delta_q;
        new_q.normalize();

        x_next.segment<QUAT_DIM>(3) =
            Eigen::Vector4d(new_q.w(), new_q.x(), new_q.y(), new_q.z());

        // Velocity remains constant (constant velocity model)
        // In practice, you might want to add damping

        return x_next;
    }

    // State transition Jacobian
    Eigen::MatrixXd stateTransitionJacobian(const Eigen::VectorXd& x) {
        Eigen::MatrixXd F = Eigen::MatrixXd::Identity(STATE_DIM, STATE_DIM);

        // Position depends on velocity
        F.block<POS_DIM, VEL_DIM>(0, 7) = Eigen::Matrix3d::Identity() * dt_;

        // Orientation Jacobian is complex - for simplicity, we approximate
        // In a full implementation, you'd compute the proper quaternion
        // Jacobian

        return F;
    }

    // Measurement function - projects state to measurement space
    Eigen::VectorXd measurementFunction(const Eigen::VectorXd& x,
                                        int marker_id) {
        Eigen::VectorXd z(MEAS_DIM -
                          1);  // Exclude ID from measurement function

        // For simplicity, we assume the state directly represents marker pose
        // In reality, you'd have transformations between robot and markers
        z.segment<POS_DIM>(0) = x.segment<POS_DIM>(0);
        z.segment<QUAT_DIM>(3) = x.segment<QUAT_DIM>(3);

        return z;
    }

    // Measurement Jacobian
    Eigen::MatrixXd measurementJacobian(const Eigen::VectorXd& x,
                                        int marker_id) {
        Eigen::MatrixXd H = Eigen::MatrixXd::Zero(MEAS_DIM - 1, STATE_DIM);

        // For simplicity, direct observation of position and orientation
        H.block<POS_DIM, POS_DIM>(0, 0) = Eigen::Matrix3d::Identity();
        H.block<QUAT_DIM, QUAT_DIM>(3, 3) = Eigen::Matrix4d::Identity();

        return H;
    }

    void updateSingleMeasurement(const Eigen::VectorXd& z_measurement) {
        int marker_id = static_cast<int>(z_measurement(0));
        Eigen::VectorXd z = z_measurement.tail(MEAS_DIM - 1);  // Remove ID

        // Measurement function and Jacobian
        Eigen::VectorXd z_pred = measurementFunction(m_x, marker_id);
        Eigen::MatrixXd H = measurementJacobian(m_x, marker_id);

        // Innovation
        Eigen::VectorXd y = z - z_pred;

        // Innovation covariance
        Eigen::MatrixXd S = H * m_P * H.transpose() + m_R;

        // Kalman gain
        Eigen::MatrixXd K = m_P * H.transpose() * S.inverse();

        // Update state and covariance
        m_x = m_x + K * y;
        m_P = (Eigen::MatrixXd::Identity(STATE_DIM, STATE_DIM) - K * H) * m_P;

        // Normalize quaternion
        auto q_vec = m_x.segment<QUAT_DIM>(3);
        Eigen::Quaterniond q(q_vec(0), q_vec(1), q_vec(2), q_vec(3));
        q.normalize();
        m_x.segment<QUAT_DIM>(3) = Eigen::Vector4d(q.w(), q.x(), q.y(), q.z());
    }

    // Helper function to convert angle-axis to quaternion
    Eigen::Quaterniond angleAxisToQuaternion(
        const Eigen::Vector3d& angle_axis) {
        double angle = angle_axis.norm();
        if (angle < 1e-10) {
            return Eigen::Quaterniond::Identity();
        }
        Eigen::Vector3d axis = angle_axis / angle;
        return Eigen::Quaterniond(Eigen::AngleAxisd(angle, axis));
    }
};

}  // namespace clover2_aruco::utils
