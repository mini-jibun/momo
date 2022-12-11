#include "servo_data_channel.h"

// WebRTC
#include <rtc_base/logging.h>

ServoDataChannel::ServoDataChannel(
    ServoDataManager* servo_data_manager,
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)
    : servo_data_manager_(servo_data_manager), data_channel_(data_channel) {
  data_channel_->RegisterObserver(this);
}

ServoDataChannel::~ServoDataChannel() {
  data_channel_->UnregisterObserver();
}

void ServoDataChannel::OnStateChange() {
  webrtc::DataChannelInterface::DataState state = data_channel_->state();
  if (state == webrtc::DataChannelInterface::kClosed) {
    servo_data_manager_->OnClosed(this);
  }
}

void ServoDataChannel::OnMessage(const webrtc::DataBuffer& buffer) {
  const uint8_t* data = buffer.data.data<uint8_t>();
  size_t length = buffer.data.size();

  servo_data_manager_->SetDegree(data, length);
}

void ServoDataChannel::Send(uint8_t* data, size_t length) {
  if (data_channel_->state() != webrtc::DataChannelInterface::kOpen) {
    return;
  }
  rtc::CopyOnWriteBuffer buffer(data, length);
  webrtc::DataBuffer data_buffer(buffer, true);
  data_channel_->Send(data_buffer);
}
