#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <utility>
#include <vector>
#include <utility>

#include "openhd-wifi.hpp"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-link-statistics.hpp"
#include "WBStreamsSettings.h"

#include "../../lib/wifibroadcast/src/UDPWfibroadcastWrapper.hpp"

/**
 * This class takes a list of discovered wifi cards (and their settings) and
 * is responsible for configuring the given cards and then setting up all the Wifi-broadcast streams needed for OpenHD.
 * This class assumes a corresponding instance on the air or ground unit, respective.
 */
class WBStreams {
 public:
  explicit WBStreams(OHDProfile profile,OHDPlatform platform,std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards);
  // register callback that is called in regular intervals with link statistics
  void set_callback(openhd::link_statistics::STATS_CALLBACK stats_callback){
	_stats_callback=std::move(stats_callback);
  }
  // Verbose string about the current state.
  [[nodiscard]] std::string createDebug() const;
  // see interface
  void addExternalDeviceIpForwarding(const std::string& ip);
  void removeExternalDeviceIpForwarding(const std::string& ip);
  // Returns true if this WBStream has ever received any data. If no data has been ever received after X seconds,
  // there most likely was an unsuccessful frequency change.
  [[nodiscard]] bool ever_received_any_data()const;
  // Some settings need a full restart of the tx / rx instances to apply
  void restart();
  // set the frequency (wifi channel) of all wifibroadcast cards
  bool set_frequency(uint32_t frequency);
  // set the tx power of all wifibroadcast cards
  bool set_txpower(uint32_t tx_power);
  // set the mcs index for all wifibroadcast cards
  bool set_mcs_index(uint32_t mcs_index);
  bool set_fec_block_length(int block_length);
  bool set_fec_percentage(int fec_percentage);
  // set the channel width
  // TODO doesn't work yet, aparently we need more than only the pcap header.
  bool set_channel_width(uint32_t channel_width);
  // settings hacky begin
  std::vector<openhd::Setting> get_all_settings()const;
  void process_new_setting(openhd::Setting changed_setting);
 private:
  const OHDProfile _profile;
  const OHDPlatform _platform;
  const int DEFAULT_MCS_INDEX = 3;
  std::vector<std::shared_ptr<WifiCardHolder>> _broadcast_cards;
 private:
  // set cards to monitor mode and set the right frequency, tx power
  void configure_cards();
  // start telemetry and video rx/tx stream(s)
  void configure_streams();
  void configure_telemetry();
  void configure_video();
  //openhd::WBStreamsSettings _last_settings;
  std::unique_ptr<openhd::WBStreamsSettingsHolder> _settings;
  // For telemetry, bidirectional in opposite directions
  std::unique_ptr<UDPWBTransmitter> udpTelemetryTx;
  std::unique_ptr<UDPWBReceiver> udpTelemetryRx;
  // For video, on air there are only tx instances, on ground there are only rx instances.
  std::vector<std::unique_ptr<UDPWBTransmitter>> udpVideoTxList;
  std::vector<std::unique_ptr<UDPWBReceiver>> udpVideoRxList;
  // TODO make more configurable
  [[nodiscard]] std::unique_ptr<UDPWBTransmitter> createUdpWbTx(uint8_t radio_port, int udp_port,bool enableFec)const;
  [[nodiscard]] std::unique_ptr<UDPWBReceiver> createUdpWbRx(uint8_t radio_port, int udp_port);
  [[nodiscard]] std::vector<std::string> get_rx_card_names()const;
  // called from the wifibroadcast instance(s), which have their own threads.
  std::mutex _statisticsDataLock;
  void onNewStatisticsData(const OpenHDStatisticsWriter::Data& data);
  // hacky, we accumulate the stats for all RX streams, which are 1 on the air (telemetry rx) and
  // 3 on the ground (telemetry and 2x video rx)
  // first is always telemetry, second and third are video if on ground
  std::array<OpenHDStatisticsWriter::Data,3> _last_stats_per_rx_stream{};
  // OpenHD
  openhd::link_statistics::STATS_CALLBACK _stats_callback=nullptr;
};

#endif
