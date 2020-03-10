#include <ros/ros.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <sensor_msgs/Range.h>
#define LED_PIN 0 // change pin number here

#include <iostream>
using namespace std;

char                 databuffer[35];
double               temp;
int                  fd;

void getBuffer()     //Get weather status data
{
  int index;
  for (index = 0;index < 35;index ++)
  {
    if(serialDataAvail(fd) )
    {
      databuffer[index] = serialGetchar(fd);
      if (databuffer[0] != 'c')
      {
        index = -1;
      }
    }
    else
    {
      index --;
    }
  }
cout << databuffer << endl;
}

int transCharToInt(char *_buffer,int _start,int _stop)                               //char to int）
{
  int _index;
  int result = 0;
  int num = _stop - _start + 1;
  int _temp[num];
  for (_index = _start;_index <= _stop;_index ++)
  {
    _temp[_index - _start] = _buffer[_index] - '0';
    result = 10*result + _temp[_index - _start];
  }
  return result;
}

int WindDirection()     //Wind Direction
{
  return transCharToInt(databuffer,1,3);
}

float WindSpeedAverage()     //air Speed (1 minute)
{
  temp = 0.44704 * transCharToInt(databuffer,5,7);
  return temp;
}

float WindSpeedMax()    //Max air speed (5 minutes)
{
  temp = 0.44704 * transCharToInt(databuffer,9,11);
  return temp;
}

int main (int argc, char **argv)
{
    ros::init(argc, argv, "anemometer_node");
    ros::NodeHandle nh;
    ros::Publisher pub_wind = nh.advertise<sensor_msgs::Range>("wind_data", 1000);

    //wiringPiSetupGpio();
    int count;
    unsigned int nextTime;

    float wind_dir;
    float wind_spd_avg;
    float wind_spd_max;

    if ((fd = serialOpen ("/dev/ttyUSB0", 9600)) < 0)
    {
        fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
        return 1 ;
    }

    while (ros::ok())
    {
        sensor_msgs::Range wind_msg;

        getBuffer();
        wind_dir = WindDirection();
        wind_spd_avg = WindSpeedAverage();
        wind_spd_max = WindSpeedMax();
//cout << wind_dir << "           " << wind_spd << endl << endl;
        wind_msg.min_range = wind_dir;
        wind_msg.max_range = wind_spd_avg;
        wind_msg.range = wind_spd_max;
        wind_msg.header.stamp = ros::Time::now();
        pub_wind.publish(wind_msg);

        ros::spinOnce();
    }

    return 0;
}
