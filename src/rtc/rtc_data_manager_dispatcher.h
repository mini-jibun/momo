#ifndef RTC_DATA_MANAGER_DISPATCHER_H_
#define RTC_DATA_MANAGER_DISPATCHER_H_

#include <vector>

#include "rtc_data_manager.h"

class RTCDataManagerDispatcher : public RTCDataManager {
 public:
  void Add(std::string label, std::shared_ptr<RTCDataManager> data_manager) {
    data_managers_map_[label].push_back(data_manager);
  }

  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override {
    std::string label = data_channel->label();
    for (std::weak_ptr<RTCDataManager> wp : data_managers_map_[label]) {
      auto data_manager = wp.lock();
      if (data_manager) {
        data_manager->OnDataChannel(data_channel);
      }
    }
    auto data_managers = data_managers_map_[label];
    data_managers.erase(
        std::remove_if(data_managers.begin(), data_managers.end(),
                       [](const std::weak_ptr<RTCDataManager>& wp) {
                         return wp.expired();
                       }),
        data_managers.end());
  }

 private:
  std::map<std::string, std::vector<std::weak_ptr<RTCDataManager>>>
      data_managers_map_;
};

#endif
