#!/bin/bash

# This file installs all the dependencies required by the realsense2_camera
# ROS package from intel's repository. The package supports running the
# D415 and D435 realsense cameras.
echo "================================================================"
echo "Configuring realsense repository and drivers."
echo "================================================================"

#sudo apt-get install -y software-properties-common
#sudo apt-key adv --keyserver keys.gnupg.net --recv-key C8B3A55A6F3EFCDE || sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-key C8B3A55A6F3EFCDE
#sudo add-apt-repository "deb http://realsense-hw-public.s3.amazonaws.com/Debian/apt-repo bionic main" -u
#sudo rm -f /etc/apt/sources.list.d/realsense-public.list
#sudo apt-get update -y
#sudo apt-get install -y librealsense2-dkms librealsense2-utils librealsense2-dev=2.16.1-0\~realsense0.13 librealsense2-dbg 


echo 'deb http://realsense-hw-public.s3.amazonaws.com/Debian/apt-repo bionic main' || sudo tee /etc/apt/sources.list.d/realsense-public.list
sudo apt-key adv --keyserver keys.gnupg.net --recv-key C8B3A55A6F3EFCDE || sudo apt-key adv --keyserver hkp://keys.gnupg.net:80 --recv-key C8B3A55A6F3EFCDE
sudo add-apt-repository "deb http://realsense-hw-public.s3.amazonaws.com/Debian/apt-repo bionic main"
sudo apt-get update -qq
sudo apt-get install librealsense2-dkms --allow-unauthenticated -y 
sudo apt-get install librealsense2-dev --allow-unauthenticated -y
sudo apt-get install librealsense2-utils --allow-unauthenticated -y
sudo apt-get install librealsense2-dbg --allow-unauthenticated -y


echo "================================================================"
echo "Finished configuring realsense. "
echo "================================================================"
