#include "servo_data_manager.h"

#include <pigpiod_if2.h>

#include <functional>
#include <iostream>

// WebRTC
#include <rtc_base/log_sinks.h>

// Boost
#include <boost/algorithm/string.hpp>

ServoDataManager::ServoDataManager(unsigned pin_x, unsigned pin_y)
    : pi_(-1), pin_x_(pin_x), pin_y_(pin_y) {}

ServoDataManager::~ServoDataManager() {
  {
    webrtc::MutexLock lock(&channels_lock_);
    for (ServoDataChannel* servo_data_channel : servo_data_channels_) {
      delete servo_data_channel;
    }
  }

  DoCloseServo();
}

void ServoDataManager::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
  webrtc::MutexLock lock(&channels_lock_);
  servo_data_channels_.push_back(new ServoDataChannel(this, data_channel));
}

void ServoDataManager::OnClosed(ServoDataChannel* servo_data_channel) {
  webrtc::MutexLock lock(&channels_lock_);
  servo_data_channels_.erase(
      std::remove(servo_data_channels_.begin(), servo_data_channels_.end(),
                  servo_data_channel),
      servo_data_channels_.end());
  delete servo_data_channel;
}

bool ServoDataManager::Initilize() {
  pi_ = pigpio_start(NULL, NULL);
  if (pi_ < 0) {
    RTC_LOG(LS_ERROR) << "pigpio_start failed";
    return false;
  }

  if (set_mode(pi_, pin_x_, PI_OUTPUT) < 0 ||
      set_mode(pi_, pin_y_, PI_OUTPUT) < 0) {
    RTC_LOG(LS_ERROR) << "pigpio set_mode failed";
    return false;
  }

  if (set_PWM_frequency(pi_, pin_x_, FREQUENCY) < 0 ||
      set_PWM_frequency(pi_, pin_y_, FREQUENCY) < 0) {
    RTC_LOG(LS_ERROR) << "pigpio set_PWM_frequency failed";
    return false;
  }

  if (set_PWM_range(pi_, pin_x_, RANGE) < 0 ||
      set_PWM_range(pi_, pin_y_, RANGE) < 0) {
    RTC_LOG(LS_ERROR) << "pigpio set_PWM_range failed";
    return false;
  }

  set_PWM_dutycycle(pi_, pin_x_, ToDutyCycle(90));
  set_PWM_dutycycle(pi_, pin_y_, ToDutyCycle(90));

  return true;
}

void ServoDataManager::SetDegree(const uint8_t* data, size_t length) {
  std::string message(data, data + length);

  RTC_LOG(LS_VERBOSE) << "Received message: " << message.c_str();

  std::vector<std::string> tokens;
  boost::split(tokens, message, boost::is_any_of(", "));
  if (tokens.size() != 2) {
    RTC_LOG(LS_ERROR) << "Malformed message: " << message.c_str();
    return;
  }

  int x = std::stoi(tokens[0]);
  x = std::max(DEGREE_X_LIMIT_MIN, std::min(DEGREE_X_LIMIT_MAX, x));
  int y = std::stoi(tokens[1]);
  y = std::max(DEGREE_Y_LIMIT_MIN, std::min(DEGREE_Y_LIMIT_MAX, y));

  if (x < 0 || x > 180 || y < 0 || y > 180) {
    RTC_LOG(LS_ERROR) << "Invalid degree: " << message.c_str();
    return;
  }

  RTC_LOG(LS_VERBOSE) << "Set degree: " << x << ", " << y;

  set_PWM_dutycycle(pi_, pin_x_, ToDutyCycle(x));
  set_PWM_dutycycle(pi_, pin_y_, ToDutyCycle(y));
}

void ServoDataManager::DoCloseServo() {
  pigpio_stop(pi_);
}
