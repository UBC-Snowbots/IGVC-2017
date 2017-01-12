/*
 * Created By: Robyn Castro
 * Created On: September 22, 2016
 * Description: The vision decision node, takes in an image from the robot's
 *              camera and produces a recommended twist message
 */
#include <VisionDecision.h>

// The constructor
VisionDecision::VisionDecision(int argc, char **argv, std::string node_name) {
    ros::init(argc, argv, node_name);

    // Setup NodeHandles
    ros::NodeHandle nh;
    ros::NodeHandle public_nh("~");

    // Setup Subscriber(s)
    std::string camera_image_topic_name = "/vision_processing/filtered_image";
    int refresh_rate = 10;
    image_subscriber = public_nh.subscribe(camera_image_topic_name, refresh_rate, &VisionDecision::imageCallBack, this);

    // Setup Publisher(s)
    std::string twist_topic = public_nh.resolveName("command");
    uint32_t queue_size = 10;
    twist_publisher = nh.advertise<geometry_msgs::Twist>(twist_topic, queue_size);


}

// This is called whenever a new message is received
void VisionDecision::imageCallBack(const sensor_msgs::Image::ConstPtr &image_scan) {
    // Deal with new messages here
    geometry_msgs::Twist twistMsg;

    // Decide how much to turn
    int relativeAngle = getDesiredAngle(image_scan->height / 2.0, image_scan);

    // Initialize linear velocities to 0
    twistMsg.linear.y = 0;
    twistMsg.linear.z = 0;

    // Initialize x and y angular velocities to 0
    twistMsg.angular.x = 0;
    twistMsg.angular.y = 0;

    // Decide how fast to move
    twistMsg.linear.x = getDesiredLinearSpeed(relativeAngle);

    // Decide how fast to turn
    twistMsg.angular.z = getDesiredAngularSpeed(relativeAngle);

    // Publish the twist message
    publishTwist(twistMsg);
}

void VisionDecision::publishTwist(geometry_msgs::Twist twist) {
    twist_publisher.publish(twist);
}

/* Functions to determine robot movement */

int VisionDecision::getDesiredAngle(double numSamples, const sensor_msgs::Image::ConstPtr &image_scan) {

    int desiredAngle = getAngleOfLine(false, numSamples, image_scan);

    // If angle coming from the left is invalid try going from the right.
    if (!(desiredAngle < 90 && desiredAngle > -90))
        desiredAngle = getAngleOfLine(true, numSamples, image_scan);

    // If both cases are invalid it will do a turn 90 degrees (Turns sharp right).
    if (!(desiredAngle < 90 && desiredAngle > -90))
        return 90;
    else
        return desiredAngle;
}

int VisionDecision::getAngleOfLine(bool rightSide, double numSamples, const sensor_msgs::Image::ConstPtr &image_scan) {

    // initialization of local variables.
    double imageHeight = image_scan->height;
    int incrementer;
    int startingPos;
    int validSamples = 0;

    // Assign garbage values
    int toParseFrom = -1;
    int topRow = -1;

    // Decides how to parse depending on if rightSide is true or false.
    if (rightSide) {
        // starts at right side then increments to the left
        startingPos = image_scan->width - 1;
        incrementer = -1;
    } else {
        // starts at left side then increments to th right
        startingPos = 0;
        incrementer = 1;
    }

    // starts parsing from the right and finds where the highest white line
    // begins.
    for (int i = 0; i < imageHeight - 1; i++) {
        int startPixel = getEdgePixel(startingPos, incrementer, i, image_scan, 1);
        if (startPixel != -1 && topRow == -1) {
            // Once found makes note of the highest point of the white line
            // and the column it starts at.
            topRow = i;
            toParseFrom = startPixel - 1;
        }
    }

    // Each slope will be compared to the top point of the highest white line
    double x1 = getMiddle(toParseFrom, topRow, rightSide, image_scan);

    double sumAngles = 0;

    // Finds slopes (corresponding to number of samples given) then returns the sum of all slopes
    // also counts how many of them are valid.
    for (double division = 0; division < numSamples && (numSamples + topRow) < imageHeight; division++) {
        double yCompared = numSamples + topRow;
        double xCompared = getMiddle(toParseFrom, (int) yCompared, rightSide, image_scan);

        double foundAngle;
        double foundSlope;

        if (xCompared == -1)
            // slope is invalid so nothing is added to sum of all angles.
            foundAngle = 0;
        else {
            // slope is valid, find the angle compared the the positive y-axis.
            foundSlope = -(xCompared - x1) / (yCompared - topRow);
            foundAngle = atan(foundSlope);
            // increment amount of valid samples
            validSamples++;
        }

        sumAngles += foundAngle;
    }

    return (int) (sumAngles / validSamples * 180.0 / M_PI); // returns the angle in degrees
}

double VisionDecision::getDesiredAngularSpeed(double desiredAngle) {
    double speedToMap = abs((int) desiredAngle);
    // the higher the desired angle, the higher the angular speed
    return mapRange(speedToMap, 0, 90, 0, 100);

}

double VisionDecision::getDesiredLinearSpeed(double desiredAngle) {
    double speedToMap = abs((int) desiredAngle);
    // the higher the desired angle the lower the linear speed.
    return 100 - mapRange(speedToMap, 0, 90, 0, 100);
}


/* Helper functions for functions that determine robot movement. */

int VisionDecision::getMiddle(int startingPos, int row, bool rightSide,
                              const sensor_msgs::Image::ConstPtr &image_scan) {

    int incrementer;
    int startPixel, endPixel;

    // Depending on chosen side, determine where to start
    // and how to iterate.
    if (rightSide)
        incrementer = -1;
    else
        incrementer = 1;

    // Find first pixel of the white line in a certain row.
    startPixel = VisionDecision::getEdgePixel(startingPos, incrementer, row, image_scan, 1);

    // Find last pixel of the white line in a certain row.
    endPixel = VisionDecision::getEdgePixel(startPixel, incrementer, row, image_scan, 0);

    // Return average of the two pixels.
    return (startPixel + endPixel) / 2;

}

int VisionDecision::getEdgePixel(int startingPos, int incrementer, int row,
                                 const sensor_msgs::Image::ConstPtr &image_scan, bool isStartPixel) {
    // Initialization of local variables
    int column = startingPos;
    int whiteVerificationCount = 0;
    int blackVerificationCount = 0;

    int edgePixel = -1;
    // Initialize these to be garbage values
    int toBeChecked = -1;

    // Find starting pixel
    while (column < image_scan->width && column >= 0) {
        // If white pixel found start verifying if proper start.
        if ((image_scan->data[row * image_scan->width + column] != 0 && isStartPixel) ||
            (image_scan->data[row * image_scan->width + column] == 0 && !isStartPixel)){
            blackVerificationCount = 0;
            // This pixel is what we are checking
            if (toBeChecked == -1)
                toBeChecked = column;

            // Determine whether toBeChecked is noise
            whiteVerificationCount++;
            if (whiteVerificationCount == NOISE_MAX)
                edgePixel = toBeChecked;
        } else {
            blackVerificationCount++;
            if (blackVerificationCount == NOISE_MAX) {
                whiteVerificationCount = 0; // Reset verification if black pixel.
                toBeChecked = -1;
            }
        }
        // Go to next element
        column += incrementer;
    }
    return edgePixel;
}


double VisionDecision::mapRange(double x, double inMin, double inMax, double outMin, double outMax) {
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}