/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#pragma once

#include "Engine.h"
#include <ableton/link/HostTimeFilter.hpp>
#include <portaudio.h>

// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

class Audio
{
public:
  Audio(ableton::Link& link);
  ~Audio();
  Engine& engine()
  {
    return m_engine;
  }

private:
  static int audioCallback(const void* inputBuffer,
    void* outputBuffer,
    unsigned long inNumFrames,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData);

  void initialize();
  void uninitialize();
  void start();
  void stop();

#ifdef __APPLE__
  ableton::link::HostTimeFilter<ableton::platforms::darwin::Clock> m_hostTimeFilter;
#else
  ableton::link::HostTimeFilter<ableton::platforms::stl::Clock> m_hostTimeFilter;
#endif

  Engine m_engine;
  double m_sampleTime;
  PaStream* m_stream;
};

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
