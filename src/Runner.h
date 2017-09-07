/*
 *  Project Muon - Main Runner Class
 *
 *  Last modified: 30/08/17
 */

#ifndef RUNNER_H
#define RUNNER_H

#define NUM_THREADS 2

#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>
#include "../include/opencv2/highgui/highgui.hpp"
#include "../include/opencv2/imgproc/imgproc.hpp"

#include <curses.h>
#include <pthread.h>
#include <time.h>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstring>

#include "../include/joystick/joystick.cc"
#include "GPIO.cpp"
#include "I2C.cpp"
#include "PinMap.h"

// Store all config values and other important global variables here
struct Sensors {
    GPIO dc_1a = GPIO(std::to_string(DC_1A));
    GPIO dc_4a = GPIO(std::to_string(DC_4A));
    GPIO dc_2a = GPIO(std::to_string(DC_2A));
    GPIO dc_3a = GPIO(std::to_string(DC_3A));
    GPIO servo = GPIO(std::to_string(SERVO));
    GPIO pir   = GPIO(std::to_string(PIR));
    void init() {
        servo.setPWMRange(SERVO_PWM_MIN, SERVO_PWM_MAX);
        dc_1a.setDirGPIO("out");
        dc_4a.setDirGPIO("out");
        dc_2a.setDirGPIO("out");
        dc_3a.setDirGPIO("out");
        servo.setDirGPIO("out");
        pir.setDirGPIO("in");
    }
    void destroy() {
        dc_1a.~GPIO();
        dc_4a.~GPIO();
        dc_2a.~GPIO();
        dc_3a.~GPIO();
    };
};

Sensors sensors;
int currCh;

Joystick joystick("/dev/input/js0");
JoystickEvent event;
int mid_t = (SERVO_PWM_MAX + SERVO_PWM_MIN) / 2.0;

// Networking config parameters
std::string IMG_DIR = "~/images/";   // Image directory to copy into
std::string USER    = "matthew";     // Host username to copy into
std::string HOST    = "10.0.0.117";  // Host IP to copy over network

#endif
