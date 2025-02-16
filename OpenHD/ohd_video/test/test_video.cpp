
#include <camera_discovery.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "ohd_video_air.h"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_link.hpp"
#include "openhd_util.h"

//
// Can be used to test / validate a camera implementation.
// R.n prints info about the received frame(s) to stdout.
// (See DummyDebugLink)
//
int main(int argc, char *argv[]) {
  // We need root to read / write camera settings.
  OHDUtil::terminate_if_not_root();
  const auto platform=DPlatform::discover();
  //auto cameras=DCameras::discover(*platform);
  auto cameras=std::vector<Camera>();
  if(cameras.empty()){
    cameras.emplace_back(createDummyCamera());
  }
  auto forwarder=SocketHelper::UDPForwarder("127.0.0.1",5600);
  auto cb=[&forwarder](int stream_index,const openhd::FragmentedVideoFrame& fragmented_video_frame){
      for(auto& fragemnt: fragmented_video_frame.frame_fragments){
        forwarder.forwardPacketViaUDP(fragemnt->data(),fragemnt->size());
      }
  };
  auto debug_link=std::make_shared<DummyDebugLink>();
  debug_link->m_opt_frame_cb=cb;
  OHDVideoAir ohdVideo(*platform,cameras, nullptr, debug_link);
  std::cout << "OHDVideo started\n";
  OHDUtil::keep_alive_until_sigterm();
  std::cerr << "OHDVideo stopped\n";
  return 0;
}
