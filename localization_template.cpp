#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <cv_bridge/cv_bridge.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <g2o/core/sparse_optimizer.h>
#include <g2o/core/block_solver.h>
#include <g2o/core/optimization_algorithm_levenberg.h>
#include <g2o/solvers/dense/linear_solver_dense.h>
#include <g2o/types/slam3d/types_slam3d.h>

using namespace std;

class ArucoG2ONode : public rclcpp::Node {
public:
    ArucoG2ONode() : Node("aruco_g2o_node"), rng(cv::getTickCount()) {

        pub_pose = this->create_publisher<geometry_msgs::msg::PoseStamped>(
            "/camera_pose", 10);

        sub_image = this->create_subscription<sensor_msgs::msg::Image>(
            "/world/aruco/model/x500_mono_cam_down_0/link/camera_link/sensor/imager/image", 10,
            std::bind(&ArucoG2ONode::imageCallback, this, std::placeholders::_1));

        pub_markers_raw = this->create_publisher<geometry_msgs::msg::PoseArray>(
    "/markers_raw", 10);

        pub_markers_opt = this->create_publisher<geometry_msgs::msg::PoseArray>(
            "/markers_optimized", 10);

        dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

        cameraMatrix =
            (cv::Mat_<double>(3, 3) << 539.936, 0, 640,
                                       0, 539.936, 480,
                                       0, 0, 1);

        distCoeffs = cv::Mat::zeros(5, 1, CV_64F);

        setupOptimizer();
    }

private:
    // ROS
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pub_pose;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr sub_image;
    rclcpp::Publisher<geometry_msgs::msg::PoseArray>::SharedPtr pub_markers_raw;
    rclcpp::Publisher<geometry_msgs::msg::PoseArray>::SharedPtr pub_markers_opt;

    // OpenCV
    cv::Ptr<cv::aruco::Dictionary> dictionary;
    cv::Mat cameraMatrix, distCoeffs;

    // g2o
    g2o::SparseOptimizer optimizer;
    vector<g2o::VertexSE3*> markers;
    g2o::VertexSE3* cam;

    // misc
    cv::RNG rng;

private:

    geometry_msgs::msg::Pose toMsg(const g2o::Isometry3& pose) {
        geometry_msgs::msg::Pose msg;

        Eigen::Quaterniond q(pose.rotation());

        msg.position.x = pose.translation().x();
        msg.position.y = pose.translation().y();
        msg.position.z = pose.translation().z();

        msg.orientation.x = q.x();
        msg.orientation.y = q.y();
        msg.orientation.z = q.z();
        msg.orientation.w = q.w();

        return msg;
    }

    // ===== utils =====
    g2o::Isometry3 MeasurementToPose(const cv::Vec3d& rvec, const cv::Vec3d& tvec) {
        cv::Mat rotationMatrix;
        cv::Rodrigues(rvec, rotationMatrix);

        g2o::Isometry3 pose = g2o::Isometry3::Identity();
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                pose.linear()(r, c) = rotationMatrix.at<double>(r, c);

        pose.translation() = Eigen::Vector3d(tvec[0], tvec[1], tvec[2]);
        return pose;
    }

    g2o::Isometry3 MakeMarkerPose(const Eigen::Vector3d& t) {
        g2o::Isometry3 pose = g2o::Isometry3::Identity();
        pose.translation() = t;
        return pose;
    }

    void addNoise(cv::Vec3d& rvec, cv::Vec3d& tvec) {
        const double rotNoiseStd = 0.05;
        const double transNoiseStd = 0.10;

        for (int i = 0; i < 3; ++i) {
            rvec[i] += rng.gaussian(rotNoiseStd);
            tvec[i] += rng.gaussian(transNoiseStd);
        }
    }

    // ===== g2o =====
    void setupOptimizer() {
        using Block = g2o::BlockSolver<g2o::BlockSolverTraits<6, 6>>;
        using LinearSolver = g2o::LinearSolverDense<Block::PoseMatrixType>;

        auto linearSolver = std::make_unique<LinearSolver>();
        auto blockSolver = std::make_unique<Block>(std::move(linearSolver));
        auto solver = new g2o::OptimizationAlgorithmLevenberg(std::move(blockSolver));

        optimizer.setAlgorithm(solver);

        markers.resize(3);
        Eigen::Vector3d positions[3] = {
            {0,0,0.001}, {1,0,0.001}, {1,1,0.001}
        };

        for (int i = 0; i < 3; ++i) {
            auto* marker = new g2o::VertexSE3();
            marker->setId(i);
            marker->setEstimate(MakeMarkerPose(positions[i]));
            marker->setFixed(true);
            optimizer.addVertex(marker);
            markers[i] = marker;
        }

        cam = new g2o::VertexSE3();
        cam->setId(1000);
        cam->setEstimate(g2o::Isometry3::Identity());
        optimizer.addVertex(cam);
    }

    // ===== callback =====
    void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg) {
        cv::Mat frame;

        try {
            frame = cv_bridge::toCvCopy(msg, "bgr8")->image;
        } catch (cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge error: %s", e.what());
            return;
        }

        vector<int> ids;
        vector<vector<cv::Point2f>> corners;

        cv::aruco::detectMarkers(frame, dictionary, corners, ids);
        cv::aruco::drawDetectedMarkers(frame, corners, ids);

        vector<g2o::EdgeSE3*> tmpEdges;
        vector<cv::Vec3d> rvecs, tvecs;

        if (!ids.empty()) {
            cv::aruco::estimatePoseSingleMarkers(
                corners, 0.3, cameraMatrix, distCoeffs, rvecs, tvecs);
            
            geometry_msgs::msg::PoseArray raw_msg;
            raw_msg.header = msg->header;

            for (size_t i = 0; i < ids.size(); ++i) {
                auto pose = MeasurementToPose(rvecs[i], tvecs[i]);
                raw_msg.poses.push_back(toMsg(pose));
            }

            pub_markers_raw->publish(raw_msg);

            for (size_t i = 0; i < ids.size(); ++i) {
                int markerId = ids[i];
                if (markerId < 0 || markerId >= (int)markers.size())
                    continue;

                // addNoise(rvecs[i], tvecs[i]);

                auto* edge = new g2o::EdgeSE3();
                edge->setVertex(0, cam);
                edge->setVertex(1, markers[markerId]);

                edge->setMeasurement(
                    MeasurementToPose(rvecs[i], tvecs[i])
                );

                edge->setInformation(Eigen::Matrix<double,6,6>::Identity());

                optimizer.addEdge(edge);
                tmpEdges.push_back(edge);
            }
        }

        if (!tmpEdges.empty()) {
            optimizer.initializeOptimization();
            optimizer.setVerbose(true);
            optimizer.optimize(3);

            geometry_msgs::msg::PoseArray opt_msg;
            opt_msg.header = msg->header;

            g2o::Isometry3 T_cam = cam->estimate();

            for (size_t i = 0; i < ids.size(); ++i) {
                int markerId = ids[i];
                if (markerId < 0 || markerId >= (int)markers.size())
                    continue;

                g2o::Isometry3 T_marker = markers[markerId]->estimate();

                // marker в системе камеры
                g2o::Isometry3 T_marker_in_cam = T_cam.inverse() * T_marker;

                opt_msg.poses.push_back(toMsg(T_marker_in_cam));
            }

            pub_markers_opt->publish(opt_msg);
        }

        for (auto* e : tmpEdges)
            optimizer.removeEdge(e);

        publishPose(msg->header);

        cv::imshow("ArUco Debug", frame);
        cv::waitKey(1);
    }

    // ===== publish =====
    void publishPose(const std_msgs::msg::Header& header) {
        auto pose = cam->estimate();

        geometry_msgs::msg::PoseStamped msg;
        msg.header = header;
        msg.header.frame_id = "world";

        Eigen::Quaterniond q(pose.rotation());

        msg.pose.position.x = pose.translation().x();
        msg.pose.position.y = pose.translation().y();
        msg.pose.position.z = pose.translation().z();

        msg.pose.orientation.x = q.x();
        msg.pose.orientation.y = q.y();
        msg.pose.orientation.z = q.z();
        msg.pose.orientation.w = q.w();

        pub_pose->publish(msg);

        cout << "Camera pos: "
             << pose.translation().transpose() << endl;
    }
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ArucoG2ONode>());
    rclcpp::shutdown();
    return 0;
}
