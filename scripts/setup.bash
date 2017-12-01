#!/bin/bash 

sudo cp $(rospack find kezbot_v2)/udev/02-kezbot.rules /etc/udev/rules.d

ROBOT_NAME=""
read -e -p "Enter robot name (default is kezbot): " -i "kezbot" ROBOT_NAME
sudo echo "export ROBOT_NAME=$ROBOT_NAME" > /etc/profile.d/kezbot.sh

if [ -d "$?" ] ; then
  printf "%s\n" "${green}Env variables has set."
else
  printf "%s\n" "${red}Setting env variables has failed, you may use with sudo."
fi
