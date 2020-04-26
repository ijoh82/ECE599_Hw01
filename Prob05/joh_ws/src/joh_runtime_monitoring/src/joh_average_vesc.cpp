#include "ros/ros.h"
#include "std_msgs/String.h"
//#include "vesc_msgs/Twist.h"
#include "vesc_msgs/VescStateStamped.h"
#include <std_msgs/Float64.h>

/* Read speed input signal */
std_msgs::Float64 speedX = 0.0;
void getSpeedX(const vesc_msgs::StateStamped sensor) {
  speedX = sensor;
}


int main(int argc, char **argv) {
  /* define filename */
  ros::init(argc, argv, "joh_average_vesc");

  ros::NodeHandle n;

  /* determine what topic to subscribe */
  ros::Subscriber sub = n.subscribe("/sensors/core", 3000, getSpeedX);

  /* determine what topic to publish */
  ros::Publisher ave_pub = n.advertise<std_msgs::String>("average_velocity", 3000);

  int freq = 5;                      // sampling speed [Hz]
  ros::Rate loop_rate(freq);

  int count = 0;
  int countSig = 0;                  // counter at keyboard input signal is received
  int countMax = 1000;               // upper limit of counting (to prevent overflow)
  int sizeWin = 10;                  // window size
  std_msgs::Float64 speedXOld = 0.0; // previous speed (to check keyboard input signal)
  std_msgs::Float64 speedXHist[10];
  std_msgs::Float64 speedXAve = 0.0;

  while (ros::ok()) {
    /* set publish format */
    std_msgs::String msg;
    std::stringstream ss;
    ss << speedXAve;
    msg.data = ss.str();
    ROS_INFO("%s", msg.data.c_str());
    
    ave_pub.publish(msg);
    ros::spinOnce();
    loop_rate.sleep();

    /* hold the velocity for 1 sec after receiving input signal */
    if (speedXOld != speedX) {
       countSig = count;
    }
    if (count > (countSig+freq-1) % countMax) {
      speedX = 0.0;
    }

    /* record velocity within one window */
    speedXHist[count % sizeWin] = speedX;

    /* calculate average velocity and update */
    if ((count % sizeWin == 0) && (count > 0)) {
      speedXAve = 0;
      for (int i=0; i<sizeWin; i++) {
        speedXAve = speedXAve + speedXHist[i];
      }
      speedXAve = speedXAve / 10;
    }

    speedXOld = speedX;
    count = (count+1) % countMax;
  }
  return 0;
}
