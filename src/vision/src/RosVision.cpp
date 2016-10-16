/*
 * Created by: Valerian Ratu
 * Created On: October 15, 2016
 * Description: Subscribes to image from a camera and publishes a binary image
 *              depending on the filter specification and setting
 * Usage: Press m to calibrate filter values, press m to save it.
 *
 * Subscribes to: /else/camera/image_raw
 * Publishes to: /vision/filtered_image
 *
 */

#include <RosVision.h>

using namespace cv;
using namespace std;

void RosVision::imageCallback(const sensor_msgs::ImageConstPtr &msg) {
    //check if enough duration has passed, ignore the error CLion is crying about
    if ((ros::Time::now() - last_published) > publish_interval) {
        last_published = ros::Time::now();
        try {
            inputImage = cv_bridge::toCvShare(msg, "bgr8")->image;
            //If input image is empty then quit
            if (!inputImage.empty()) {
                inputImage.copyTo(workingImage);

                //Applies the IPM to the image
                ipm.applyHomography(workingImage, ipmOutput);

                //Draw points on the original image
                ipm.drawPoints(orig_points, workingImage);

                //Filters the image
                filter.filterImage(ipmOutput, filterOutput);

                //Shows and updates images if debugging
                if (showWindow) {
                    createWindow();
                    imshow(inputWindow, workingImage);
                    imshow(ipmOutputWindow, ipmOutput);
                    imshow(filterOutputWindow, filterOutput);
                } else {
                    //destroyAllWindows();
                }

                //Outputs the image
                sensor_msgs::ImagePtr output_message = cv_bridge::CvImage(std_msgs::Header(), "mono8",
                                                                          filterOutput).toImageMsg();
                pub.publish(output_message);

                //Color filter calibration
                if (isCalibratingManually) {
                    filter.manualCalibration();
                } else {
                    filter.stopManualCalibration();
                }

                //Escape key to finish program
                int a = waitKey(20);
                if (a == 27) {
                    ROS_INFO("Escaped by user");
                    ros::shutdown();
                }
                    //Press 'm' to calibrate manually
                else if (a == 109) {
                    if (!isCalibratingManually) {
                        ROS_INFO("Beginning manual calibration");
                    } else {
                        ROS_INFO("Ending manual calibration");
                    }
                    isCalibratingManually = !isCalibratingManually;
                    filter.printValues();
                }
                    //Press 's' to show/unshow window
                else if (a == 115) {
                    showWindow = !showWindow;
                }
            }
        }
        catch (cv_bridge::Exception &e) {
            ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
        }
    }
}

void RosVision::createWindow() {
    namedWindow(inputWindow, CV_WINDOW_AUTOSIZE);
    namedWindow(ipmOutputWindow, CV_WINDOW_AUTOSIZE);
    namedWindow(filterOutputWindow, CV_WINDOW_AUTOSIZE);
}

void RosVision::deleteWindow() {
    destroyWindow(inputWindow);
    destroyWindow(ipmOutputWindow);
    destroyWindow(filterOutputWindow);
}

RosVision::RosVision() {
}


RosVision::RosVision(int argc, char **argv, std::string node_name) {

    //Window Names
    inputWindow = "Input Image";
    ipmOutputWindow = "IPM Output";
    filterOutputWindow = "Filter Output";

    //Calibration Variables
    showWindow = true;
    isCalibratingManually = false;

    inputImage = Mat::zeros(480, 640, CV_32FC4);

    //ROS
    ros::init(argc, argv, node_name);
    ros::NodeHandle nh;
    ros::NodeHandle nh_private("~");

    //Finds the image and output topics
    nh_private.param<std::string>("image_topic", image_topic, "/elsa/camera/image_raw");
    nh_private.param<std::string>("output_topic", output_topic, "/vision/filtered_image");
    ROS_INFO("Image (Subscribe) Topic: %s", image_topic.c_str());
    ROS_INFO("Output (Publish) Topic: %s", output_topic.c_str());

    //Initializes publishers and subscribers
    image_transport::ImageTransport it(nh);
    sub = it.subscribe<RosVision>(image_topic, 1, &RosVision::imageCallback, this);
    pub = it.advertise(output_topic, 1);

    //Obtains parameters of image and filters from the param server
    nh_private.param("width", width, 640);
    nh_private.param("height", height, 480);
    nh_private.param("x1", x1, 0);
    nh_private.param("x2", x2, width);
    nh_private.param("x3", x3, width / 2 + 132);
    nh_private.param("x4", x4, width / 2 - 132);
    nh_private.param("y1", y1, height);
    nh_private.param("y2", y2, height);
    nh_private.param("y3", y3, 0);
    nh_private.param("y4", y4, 0);

    double frequency;
    nh_private.param("frequency", frequency, 5.0);
    publish_interval = ros::Duration(1 / frequency);
    last_published = ros::Time::now();


    ROS_INFO("Image Width and Height: %d x %d", width, height);


    //Manually Setting up the IPM points
    orig_points.push_back(Point2f(x1, y1));
    orig_points.push_back(Point2f(x2, y2));
    orig_points.push_back(Point2f(x3, y3));
    orig_points.push_back(Point2f(x4, y4));

    dst_points.push_back(Point2f(0, height));
    dst_points.push_back(Point2f(width, height));
    dst_points.push_back(Point2f(width, 0));
    dst_points.push_back(Point2f(0, 0));

    //Creating the binary filter
    filter = snowbotsFilter(0, 155, 0, 155, 150, 255);

    //Creating the IPM transformer
    ipm = IPM(Size(width, height), Size(width, height), orig_points, dst_points);

}
