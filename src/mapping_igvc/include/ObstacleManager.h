/*
 * Created By: Gareth Ellis
 * Created On: January 9, 2018
 * Description: The Obstacle Manager takes in discrete obstacles and saves them,
 *              comparing them to newly received obstacles and checking if
 *              they're similar. If they are, it will merge/move known obstacles
 *              using the new obstacles as updated information
 */

#ifndef MAPPING_IGVC_OBSTACLEMANAGER_H
#define MAPPING_IGVC_OBSTACLEMANAGER_H

// C++ STD includes
#include <vector>

// Snowbots Includes
#include <sb_geom/Spline.h>
#include <sb_geom/Polynomial.h>
#include <sb_geom/PolynomialSegment.h>
#include <mapping_igvc/ConeObstacle.h>
#include <mapping_igvc/LineObstacle.h>

// ROS Includes
#include <nav_msgs/OccupancyGrid.h>

class ObstacleManager {
public:

    /**
     * We delete the default constructor to force the user of this class to use
     * one of the other provided constructors
     */
    ObstacleManager() = delete;

    /**
     * Creates an ObstacleManager
     *
     * @param cone_merging_tolerance the minimum distance between the center of
     * two cones for them to be considered different from each other
     * (and so not merged)
     * @param line_merging_tolerance the minimum distance (measured at the
     * closest point) for two lines to be considered different
     * (and so not merged)
     */
    explicit ObstacleManager(double cone_merging_tolerance, double line_merging_tolerance):
        ObstacleManager(
                cone_merging_tolerance,
                line_merging_tolerance,
                0,
                0.1
        ) {};

    /**
     * Creates a ObstacleManager
     *
     * @param cone_merging_tolerance the minimum distance between the center of
     * two cones for them to be considered different from each other
     * (and so not merged)
     * @param line_merging_tolerance the minimum distance (measured at the
     * closest point) for two lines to be considered different
     * (and so not merged)
     * @param obstacle_inflation_buffer the buffer to add around each obstacle
     * when generating an occupancy grid for the the world
     * @param occ_grid_cell_size the length/width of every cell in the
     * occupany grids generated by `generateOccupancyGrid`
     */
    explicit ObstacleManager(double cone_merging_tolerance,
                             double line_merging_tolerance,
                             double obstacle_inflation_buffer,
                             double occ_grid_cell_size
    ): ObstacleManager(
            cone_merging_tolerance,
            line_merging_tolerance,
            obstacle_inflation_buffer,
            occ_grid_cell_size,
            10,
            15
    ) {};

    /**
     * Creates a ObstacleManager
     *
     * @param cone_merging_tolerance the minimum distance between the center of
     * two cones for them to be considered different from each other
     * (and so not merged)
     * @param line_merging_tolerance the minimum distance (measured at the
     * closest point) for two lines to be considered different
     * (and so not merged)
     * @param obstacle_inflation_buffer the buffer to add around each obstacle
     * when generating an occupancy grid for the the world
     * @param occ_grid_cell_size the length/width of every cell in the
     * occupany grids generated by `generateOccupancyGrid`
     * @param line_merging_max_iters the maximum number of iterations to use
     * when merging lines. larger values will make line merging slower,
     * but more accurate
     * @param closest_line_max_iters the maximum number of iterations to use
     * when comparing a new line to known lines. Larger values will make
     * this more accurate (ie. we are more likely to find the truly closest
     * known line), but slower
     */
    explicit ObstacleManager(double cone_merging_tolerance,
                             double line_merging_tolerance,
                             double obstacle_inflation_buffer,
                             double occ_grid_cell_size,
                             unsigned int line_merging_max_iters,
                             unsigned int closest_line_max_iters
    );

    /**
     * Add a given cone to our map of the world
     *
     * Assumes that given obstacle is already in frame of generated occ grid
     *
     * @param cone the cone to add
     */
    void addObstacle(mapping_igvc::ConeObstacle cone);

    /**
     * Add a given line to our map of the world
     *
     * Assumes that given obstacle is already in frame of generated occ grid
     *
     * @param line_obstacle the line to add
     */
    void addObstacle(mapping_igvc::LineObstacle line_obstacle);

    /**
     * Add a given spline to our map of the world
     *
     * @param line_obstacle the line to add
     */
    void addObstacle(sb_geom::Spline spline);

    /**
     * Get all the cones in our world
     *
     * Assumes that given obstacle is already in frame of generated occ grid
     *
     * @return a list of all known cones in our world
     */
    std::vector<mapping_igvc::ConeObstacle> getConeObstacles();

    /**
     * Get all the lines in our world
     *
     * @return a list of all known lines in our world
     */
    std::vector<sb_geom::Spline> getLineObstacles();

    /**
     * Generates an occupancy grid with all known obstacles
     *
     * Does *NOT* set `header` or `info.map_load_time`, this is the responsiblity of the caller
     *
     * @return an occupancy grid containing all known obstacles
     */
    nav_msgs::OccupancyGrid generateOccupancyGrid();

    /**
     * Inflates the given point (`point`) in `occ_grid` in a circle with radius: `inflation_radius`
     *
     * This will set all grid cells within `inflation_radius` of `point` to 100
     * @param occ_grid
     * @param point
     * @param inflation_radius
     */
    static void inflatePoint(nav_msgs::OccupancyGrid& occ_grid, sb_geom::Point2D point, double inflation_radius);

private:

    /**
     * Merges the new line into the current line to come up with an updated line
     *
     * Prioritizes new_line over current_line when merging
     *
     * @param current_line our currently known line
     * @param new_line the newly found line
     * @return the merged line
     */
    sb_geom::Spline updateLineWithNewLine(sb_geom::Spline current_line,
                                          sb_geom::Spline new_line);

    // the minimum distance between the center of two cones for them to be
    // considered different from each other (and so not merged)
    double cone_merging_tolerance;

    // the minimum distance between two lines for them to be
    // considered different from each other (and so not merged)
    double line_merging_tolerance;

    // the buffer to add around each obstacle when generating the occupancy
    // grid for our currently known world
    double obstacle_inflation_buffer;

    // the size (length/width) of the cells in occupancy grids this class
    // generates
    double occ_grid_cell_size;

    // the maximum number of iterations to use when merging splines
    // larger values will make line merging slower, but more accurate
    unsigned int spline_merging_max_iters;

    // the maximum number of iterations to use when comparing a new spline
    // to known splines. Larger values will make this more accurate (ie. we are
    // more likely to find the truly closest known spline), but slower
    unsigned int closest_spline_max_iters;

    // all known cones in our world
    std::vector<mapping_igvc::ConeObstacle> cones;

    // all known lines in our world
    std::vector<sb_geom::Spline> lines;

};


#endif //MAPPING_IGVC_OBSTACLEMANAGER_H