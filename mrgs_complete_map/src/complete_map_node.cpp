/*********************************************************************
*
* Software License Agreement (BSD License)
*
*  Copyright (c) 2014, ISR University of Coimbra.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the ISR University of Coimbra nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
* Author: Gonçalo S. Martins, 2014
*********************************************************************/

/** 
 * complete_map_node
 * 
 * Summary:
 * 
 * Methodology:
 *  1. Receive maps
 *  3. Send them two by two to be aligned
 *  4. Publish the complete map
 * 
 */
// ROS includes
#include "ros/ros.h"
#include "mrgs_alignment/align.h"
#include "mrgs_data_interface/ForeignMapVector.h"
#include <cstdlib>

// Global variables
// To be edited only by the /foreign_maps callback
std::vector<ros::Time> g_latest_map_times;
// To be edited only by the processForeignMaps callback
// Keeps the "dirtiness" (the need to be rebuilt) of aligned maps.
std::vector<std::vector<bool> > g_is_dirty;
// To be edited by the processForeignMaps callback
std::vector<std::vector<nav_msgs::OccupancyGrid> > g_aligned_maps;
// To allow calls to service from callbacks
ros::ServiceClient client;

void processForeignMaps(const mrgs_data_interface::ForeignMapVector::ConstPtr& maps)
{
  /// Inform
  ROS_INFO("Received a foreign map vector with %d maps.", maps->map_vector.size());
  g_is_dirty.clear(); // This variable will later become local.
  
  /// Allocate dirtiness matrix
  ROS_INFO("Allocating dirtiness matrix...");
  // Allocate first row
  g_is_dirty.push_back(std::vector<bool>(maps->map_vector.size(), true));
  // Allocate subsequent rows:
  ROS_DEBUG("First row allocated. Allocating others...");
  int i = 0;
  do
  {
    i++;
    int prev_n = g_is_dirty.at(i-1).size();
    int curr_n;
    prev_n % 2 == 0? curr_n = prev_n/2:curr_n = (prev_n+1)/2;
    g_is_dirty.push_back(std::vector<bool>(curr_n, true));
    ROS_DEBUG("Allocated a new row with %d elements.", curr_n);
  }while(g_is_dirty.at(i).size() > 1);
  ROS_DEBUG("Allocated %d new rows.", i);
  
  /// Expand aligned map matrix (if needed)
  std::vector<nav_msgs::OccupancyGrid> empty_vec;
  nav_msgs::OccupancyGrid empty_map;
  while(g_aligned_maps.size() < g_is_dirty.size())
  {
    g_aligned_maps.push_back(empty_vec);
  }
  for(int i = 0; i < g_is_dirty.size()-1; i++)
  {
    while(g_aligned_maps.at(i).size() < g_is_dirty.at(i-1).size())
      g_aligned_maps.at(i).push_back(empty_map);
  }
  
  /// Check if the received maps have updates and mark as dirty accordingly
  if(g_latest_map_times.size()==0)
  {
    // There is no previous data, we're on the first run
    for(int i = 0; i < maps->map_vector.size(); i++)
    {
      g_latest_map_times.push_back(maps->map_vector.at(i).map.header.stamp);
    }
  }
  else
  {
    for(int i = 0; i < g_is_dirty.at(0).size(); i++)
    {
      if(g_latest_map_times.at(i) < maps->map_vector.at(i).map.header.stamp)
      {
        g_is_dirty.at(0).at(i) = true;
        ROS_INFO("Map %d is dirty.", i);
      }
      else
        g_is_dirty.at(0).at(i) = false;
    }
  }
  
  /// (Re-)Build maps
  // Iterate through the dirtiness matrix, starting in row 1 (not 0), and rebuild 
  // all maps which depend on a "dirty" map. Newly-built maps are also marked as "dirty".
  // This process is repeated until we only have a single map to build, which will be our final map.
  for(int i = 1; g_is_dirty.at(i).size() > 1; i++)
  {
    for(int j = 0; j < g_is_dirty.at(i).size(); j++)
    {
      // How many maps does this one depend on?
      if(2*j == g_is_dirty.at(i-1).size() - 1)
      {
        // This map only depends on one other
      }
      else
      {
        // This map depends on two others.
        // Are any of them dirty?
      }
    }
  }
  
  /// Publish our new, shiny, complete map
  // Use a latched topic, so that the last map is accessible to new subscribers.
}

int main(int argc, char **argv)
{
  // ROS initialization
  ros::init(argc, argv, "complete_map_node");
  ros::NodeHandle n;
  client = n.serviceClient<mrgs_alignment::align>("align");
  mrgs_alignment::align srv;
  ros::Subscriber sub2 = n.subscribe("foreign_maps", 1, processForeignMaps);
  ros::Publisher pub1 = n.advertise<nav_msgs::OccupancyGrid>("complete_map", 10);
  
  
  //ros::Rate r(1/30.0);
  
  // ROS loop
  ros::spin();
  /*while(ros::ok())
  {
    // Get all maps
    ros::spinOnce();
    /*
    if (client.call(srv))
    {
        
    }
    else
    {
      ROS_ERROR("Service call failed (probably no occupied cells in one of the supplied grids).");
      return 1;
    }
    r.sleep();
  }*/

  
  return 0;
}
