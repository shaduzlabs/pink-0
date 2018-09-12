/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#pragma once

#include "Engine.h"
#include <ableton/platforms/Config.hpp>
#include <ableton/link/HostTimeFilter.hpp>

#ifdef AUDIO_USE_RTAUDIO
#include <RtAudio.h>
#else
#include <portaudio.h>
#endif
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
#ifdef AUDIO_USE_RTAUDIO
  static int audioCallbackRTA(void* outputBuffer,
    void* inputBuffer,
    unsigned int nBufferFrames,
    double streamTime,
    RtAudioStreamStatus status,
    void* userData);
#else
  static int audioCallbackPA(const void* inputBuffer,
    void* outputBuffer,
    unsigned long inNumFrames,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData);
#endif

  void initialize();
  void uninitialize();
  void start();
  void stop();

  ableton::link::HostTimeFilter<ableton::link::platform::Clock> m_hostTimeFilter;

  Engine m_engine;
  double m_sampleTime;

#ifdef AUDIO_USE_RTAUDIO
  RtAudio m_audioDevice;
  unsigned m_audioBufferSize;
  RtAudio::StreamParameters m_audioStreamParameters;
#else
  PaStream* m_stream;
#endif
};

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
