#include "servo_data_manager.h"

#include <pigpiod_if2.h>

#include <functional>
#include <iostream>

// WebRTC
#include <rtc_base/log_sinks.h>

// Boost
#include <boost/algorithm/string.hpp>

ServoDataManager::ServoDataManager(unsigned pin_roll, unsigned pin_pitch)
    : pi_(-1), pin_roll_(pin_roll), pin_pitch_(pin_pitch) {}

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

  if (set_mode(pi_, pin_roll_, PI_OUTPUT) < 0 ||
      set_mode(pi_, pin_pitch_, PI_OUTPUT) < 0) {
    RTC_LOG(LS_ERROR) << "pigpio set_mode failed";
    return false;
  }

  if (set_PWM_frequency(pi_, pin_roll_, FREQUENCY) < 0 ||
      set_PWM_frequency(pi_, pin_pitch_, FREQUENCY) < 0) {
    RTC_LOG(LS_ERROR) << "pigpio set_PWM_frequency failed";
    return false;
  }

  if (set_PWM_range(pi_, pin_roll_, RANGE) < 0 ||
      set_PWM_range(pi_, pin_pitch_, RANGE) < 0) {
    RTC_LOG(LS_ERROR) << "pigpio set_PWM_range failed";
    return false;
  }

  set_PWM_dutycycle(pi_, pin_roll_, ToDutyCycle(90));
  set_PWM_dutycycle(pi_, pin_pitch_, ToDutyCycle(90));

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

  int roll = std::stoi(tokens[0]);
  roll = std::max(DEGREE_X_LIMIT_MIN, std::min(DEGREE_X_LIMIT_MAX, roll));
  int pitch = std::stoi(tokens[1]);
  pitch = std::max(DEGREE_Y_LIMIT_MIN, std::min(DEGREE_Y_LIMIT_MAX, pitch));

  if (roll < 0 || roll > 180 || pitch < 0 || pitch > 180) {
    RTC_LOG(LS_ERROR) << "Invalid degree: " << message.c_str();
    return;
  }

  RTC_LOG(LS_VERBOSE) << "Set degree: " << roll << ", " << pitch;

  set_PWM_dutycycle(pi_, pin_roll_, ToDutyCycle(roll));
  set_PWM_dutycycle(pi_, pin_pitch_, ToDutyCycle(pitch));
}

void ServoDataManager::DoCloseServo() {
  pigpio_stop(pi_);
}
