/*
 * Created By: Valerian Ratu
 * Created On: January 29, 2017
 * Description: Integration testing for MyNode
 */


#include <MyNode.h>
#include <gtest/gtest.h>
#include <ros/ros.h>


/**
 * This is the helper class which will publish and subscribe messages which will test the node being instantiated
 * It contains on the minimum:
 *      publisher - publishes the input to the node
 *      subscriber - publishes the output of the node
 *      callback function - the callback function which corresponds to the subscriber
 *      getter function - to provide a way for gtest to check for equality of the message recieved
 */
class MyClassTest : public testing::Test{
protected:
    virtual void SetUp(){
        testPublisher = nh_.advertise<std_msgs::String>("subscribe_topic", 1);
        testSubscriber = nh_.subscribe("/my_node/publish_topic", 1, &MyClassTest::callback, this);

        // Let the publishers and subscribers set itself up timely
        ros::Rate loop_rate(1);
        loop_rate.sleep();
    }

    ros::NodeHandle nh_;
    std::string messageOutput;
    ros::Publisher testPublisher;
    ros::Subscriber testSubscriber;

public:

    void callback(const std_msgs::String::ConstPtr msg){
        messageOutput = msg->data.c_str();
    }

    std::string getMessage(){
        return messageOutput;
    }
};

TEST_F(MyClassTest, MyNodeTest){

    // publishes "Hello" to the test node
    std_msgs::String msg;
    msg.data = "Hello";
    testPublisher.publish(msg);

    // Wait for the message to get passed around
    ros::Rate loop_rate(1);
    loop_rate.sleep();

    // spinOnce allows ros to actually process your callbacks
    // for the curious:
    //      http://answers.ros.org/question/11887/significance-of-rosspinonce/
    ros::spinOnce();

    EXPECT_EQ("Hello!", getMessage());
}


int main(int argc, char **argv) {
    // !! Don't forget to initialize ROS, since this is a test within the ros framework !!
    ros::init(argc, argv, "my_node_rostest");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}