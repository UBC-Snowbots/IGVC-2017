/*
 * Created By: Valerian Ratu
 * Created On: May 1 2017
 * Description: A node which transforms a given pointcloud from
 *              it's current frame to a given frame
 */

#include <geometry_msgs/TransformStamped.h>
#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_sensor_msgs/tf2_sensor_msgs.h>

#include <sb_utils.h>

ros::Publisher pub;
tf2_ros::Buffer tf_buffer;

std::string output_frame;
ros::Time last_exception_time;

// The amount of time(s) for a new pointcloud transform to be published.
double transform_period;

void pointCloudCallback(const sensor_msgs::PointCloud2::ConstPtr& msg) {
    try {
        // Create an empty pointcloud
        sensor_msgs::PointCloud2 output;
        // Transform the pointcloud to the requested frame
        geometry_msgs::TransformStamped tf_stamped = tf_buffer.lookupTransform(
        msg->header.frame_id, output_frame, ros::Time(0), ros::Duration(0.1));

        tf2::doTransform(*msg, output, tf_stamped);

        output.header.frame_id = output_frame;

        // Publish the transformed pointcloud
        pub.publish(output);

        ros::Duration(transform_period).sleep();
    } catch (tf2::TransformException ex) {
        if (ros::Time::now().toSec() - last_exception_time.toSec() > 1.0)
            ROS_WARN("%s", ex.what());

        last_exception_time = ros::Time::now();
    }
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "pointcloud_transformer");
    ros::NodeHandle private_nh("~");
    if (!SB_getParam(private_nh, "output_frame", output_frame)) {
        // Error and exit if we didn't get a frame to transform to.
        // We need this to transform anything, and there is no reasonable
        // default
        ROS_ERROR(
        "Param 'output_frame' not provided. "
        "Can't  transform anything without a frame to transform it to");
        return 1;
    }

    double default_transform_period = 0;
    SB_getParam(
    private_nh, "transform_period", transform_period, default_transform_period);

    ros::Subscriber sub =
    private_nh.subscribe("/input_pointcloud", 1, pointCloudCallback);
    pub =
    private_nh.advertise<sensor_msgs::PointCloud2>("/output_pointcloud", 1);
    tf2_ros::TransformListener tfListener(tf_buffer);
    ros::spin();
    return 0;
}
