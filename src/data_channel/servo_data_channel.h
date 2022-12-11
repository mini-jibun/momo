#ifndef SERVO_DATA_CHANNEL_H_
#define SERVO_DATA_CHANNEL_H_

// WebRTC
#include <api/data_channel_interface.h>

#include "servo_data_manager.h"

class ServoDataManager;

class ServoDataChannel : public webrtc::DataChannelObserver {
 public:
  ServoDataChannel(
      ServoDataManager* servo_data_manager,
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel);
  ~ServoDataChannel();

  void Send(uint8_t* data, size_t length);

  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer& buffer) override;
  void OnBufferedAmountChange(uint64_t previous_amount) override {}

 private:
  ServoDataManager* servo_data_manager_;
  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
};

#endif
