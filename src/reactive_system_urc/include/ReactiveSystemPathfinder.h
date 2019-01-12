/**
 * Created by William Gu on Nov 3 2018
 * Declarations for Reactive System Pathfinder
 * This node is responsible for moving the robot towards a given GPS goal while avoiding risky areas
 */


#ifndef PROJECT_REACTIVESYSTEMPATHFINDER_H
#define PROJECT_REACTIVESYSTEMPATHFINDER_H

#include <iostream>
#include <math.h>
#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Point32.h>
#include <geometry_msgs/Pose.h>
#include <mapping_msgs_urc/RiskArea.h>
#include <mapping_msgs_urc/RiskAreaArray.h>
#include <mapping_msgs_urc/RiskAreaStamped.h>
#include <std_msgs/Float64.h>



class ReactiveSystemPathfinder {
public:

    /**
     * Entry point for pathfinding algorithm, which produces an immediate twist message
     * @param risk_areas
     * @param goal_pos
     * @param traj_time_inc
     * @param traj_num_incs
     * @param linear_vel
     * @param max_angular_vel
     * @param num_angular_vel
     * @param risk_dist_tol_sq
     * @return
     */
    static geometry_msgs::Twist pathFinder(mapping_msgs_urc::RiskAreaArray risk_areas, geometry_msgs::Point32 goal_pos, float traj_time_inc, int traj_num_incs, float linear_vel, float max_angular_vel, int num_angular_vel, float risk_dist_tol_sq);

    /**
     * Calculates the arc trajectory given some twist command (linear and angular velocity), form of trajectory is point locations of the trajectory
     * given at discrete time steps , and coordinates are in the frame of robot position
     * @param linear_vel: linear x velocity of trajectory
     * @param angular_vel: angular z velocity of trajectory
     * @param time_inc: time increment step in seconds, dictates the distance between trajectory points (should not need to be too small)
     * @param num_incs: number of point positions in trajectory (should not be too high but >= 1)
     * @return list of point positions
     */
    static std::vector<geometry_msgs::Point32> getArcTrajectory(float linear_vel, float angular_vel, float time_inc, int num_incs);

    /**
     * Calculate the arc point in a given arc trajectory
     * @ param center: center point to rotate origin about
     * @ param angle: amount to rotate point by, in radians
     * @return point position in arc
     */
    static geometry_msgs::Point32 getArcPoint(geometry_msgs::Point32 center, float angle);

    /**
     * Calculate the risk score for a given trajectory, where higher scores are riskier
     * @param trajectory: a proposed trajectory for the robot
     * @param goal_pos: the current goal heading for the robot (in robot frame)
     * @param risk_areas
     * @param dist_tol_sq: the maximum distance for any given risk area to be within some trajectory point (should change based on robot dimensions), SQUARED
     * @return a risk score for the given trajectory
     */
    static float getTrajectoryScore(std::vector<geometry_msgs::Point32> trajectory, geometry_msgs::Point32 goal_pos, mapping_msgs_urc::RiskAreaArray risk_areas, float dist_tol_sq);

    /**
     * Helper for calcTrajectoryScore, calculates if two points are within some distance of another and returns true if so
     * @param dist_tol_sq: distance tolerance, squared
     * @return
     */
    static bool isWithinDistance(geometry_msgs::Point32 p1, geometry_msgs::Point32 p2, float dist_tol_sq);
};

#endif //PROJECT_REACTIVESYSTEMPATHFINDER_H
