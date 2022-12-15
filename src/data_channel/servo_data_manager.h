#ifndef SERVO_DATA_MANAGER_H_
#define SERVO_DATA_MANAGER_H_

#include <memory>
#include <vector>

// Boost
#include <boost/asio.hpp>

// WebRTC
#include <rtc_base/synchronization/mutex.h>

#include "rtc/rtc_data_manager.h"
#include "servo_data_channel.h"

class ServoDataChannel;

class ServoDataManager : public RTCDataManager {
 private:
  static constexpr unsigned RANGE = 40000;
  static constexpr unsigned FREQUENCY = 50;
  static constexpr unsigned PULSE_MIN = 500;
  static constexpr unsigned PULSE_MAX = 2500;

  static constexpr int DEGREE_MIN = 0;
  static constexpr int DEGREE_MAX = 180;
  static constexpr int DEGREE_MID = DEGREE_MAX / 2;

  static constexpr int DEGREE_X_LIMIT_MIN = 20;
  static constexpr int DEGREE_X_LIMIT_MAX = 180;

  static constexpr int DEGREE_Y_LIMIT_MIN = 0;
  static constexpr int DEGREE_Y_LIMIT_MAX = 180;

  static constexpr unsigned DUTY_CYCLE_MIN =
      (RANGE * PULSE_MIN) / (1000000 / FREQUENCY);
  static constexpr unsigned DUTY_CYCLE_MAX =
      (RANGE * PULSE_MAX) / (1000000 / FREQUENCY);

  unsigned ToDutyCycle(int degree) {
    return (degree - DEGREE_MIN) * (DUTY_CYCLE_MAX - DUTY_CYCLE_MIN) /
               (DEGREE_MAX - DEGREE_MIN) +
           DUTY_CYCLE_MIN;
  }

 public:
  static std::unique_ptr<ServoDataManager> Create(unsigned pin_x,
                                                  unsigned pin_y) {
    std::unique_ptr<ServoDataManager> data_manager(
        new ServoDataManager(pin_x, pin_y));
    if (!data_manager->Initilize()) {
      return nullptr;
    }
    return data_manager;
  }
  ~ServoDataManager();

  void SetDegree(const uint8_t* data, size_t length);

  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;
  void OnClosed(ServoDataChannel* servo_data_channel);

  std::string label() { return "servo"; }

 private:
  ServoDataManager(unsigned pin_x, unsigned pin_y);
  bool Initilize();
  void DoCloseServo();

  webrtc::Mutex channels_lock_;
  std::vector<ServoDataChannel*> servo_data_channels_;

  int pi_;
  unsigned pin_x_;
  unsigned pin_y_;
};

#endif
