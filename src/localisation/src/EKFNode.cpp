/*
 * Created By: Marcus Swift
 * Created On: November 20th, 2017
 * Description: An extended kalman filter node that takes in sensor data from
 * the GPS, encoders and IMU and returns the bots estimated postion and
 * orientation using the ekf class
 */

#include <EKFNode.h>
#include <EKF.h>

EKFNode::EKFNode(int argc, char** argv, std::string node_name) {
    // Setup NodeHandle
    ros::init(argc, argv, node_name);
    ros::NodeHandle nh;

    // Setup Subscribers
    gps_sub     = nh.subscribe("/gps_driver/odom", 10, &EKFNode::gpsCallBack, this);
    encoder_sub = nh.subscribe("/odom", 10, &EKFNode::encoderCallBack, this);
    imu_sub     = nh.subscribe("/imu", 10, &EKFNode::imuCallBack, this);

    // Setup Publisher
    pose_pub = nh.advertise<geometry_msgs::Pose>("/cmd_pose", 10);

    // set initial position
    bot_position.position.x    = intial_pos_x;
    bot_position.position.y    = intial_pos_y;
    bot_position.position.z    = intial_pos_z;
    bot_position.orientation.x = intial_ori_x;
    bot_position.orientation.y = intial_ori_y;
    bot_position.orientation.z = intial_ori_z;
    bot_position.orientation.w = intial_ori_w;

    // initial time
    time = ros::Time::now().toSec();
}

void EKFNode::gpsCallBack(const nav_msgs::Odometry::ConstPtr& gps_message) {
	// Gather GPS data currently, assume gps updates slower than encoder and imu
	gps_data.pose  = gps_message->pose;
	gps_data.twist = gps_message->twist;

    publishPose(measurementModel(gps_data, imu_data));
}

void EKFNode::encoderCallBack(const nav_msgs::Odometry::ConstPtr& encoder_message) {
    // Gather encoder data, currently assume encoder updates slower than imu
    encoder_data.pose  = encoder_message->pose;
    encoder_data.twist = encoder_message->twist;

    double dt = ros::Time::now().toSec() - time; //set change in time to 1
    // for rostest
    time += dt;
	
	processModel(encoder_data, imu_data, dt);
}

void EKFNode::imuCallBack(const sensor_msgs::Imu::ConstPtr& imu_message) {
    // Gather IMU data
    imu_data.orientation         = imu_message->orientation;
    imu_data.angular_velocity    = imu_message->angular_velocity;
    imu_data.linear_acceleration = imu_message->linear_acceleration;
}

void EKFNode::publishPose(geometry_msgs::Pose pose_msg) {
    // publish bot's position
	geometry_msgs::Pose test_position;
	test_position.position.x    = 99;
    test_position.position.y    = 99;
    test_position.position.z    = 99;
    test_position.orientation.x = 0;
    test_position.orientation.y = 0;
    test_position.orientation.z = 0;
    test_position.orientation.w = 1;
	std::cout << "published msg " << pose_msg << std::endl;

    pose_pub.publish(pose_msg);
}