/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#pragma once

#include <ableton/Link.hpp>
#include <mutex>

// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

class Engine
{
public:
  using Latency = std::chrono::microseconds;
  using AudioBuffer = std::vector<double>;

  Engine(ableton::Link& link);
  void startPlaying();
  void stopPlaying();
  bool isPlaying() const;

  std::size_t bufferSize() const;
  void setBufferSize(std::size_t size);

  double sampleRate() const;
  void setSampleRate(double sampleRate);

  double beatTime() const;
  double tempo() const;
  void setTempo(double tempo);
  double quantum() const;
  void setQuantum(double quantum);
  double clockMultiplier() const;
  void setClockMultiplier(double);

  const Latency& outputLatency() const;
  void setOutputLatency(Latency /*latency_*/);

  const AudioBuffer& bufferClock() const;
  const AudioBuffer& bufferReset() const;

  void process(const std::chrono::microseconds hostTime, std::size_t numSamples);

private:
  struct Data
  {
    double requestedTempo;
    bool resetBeatTime;
    bool isPlaying;
    double quantum;
  };

  Data pullEngineData();
  void renderMetronomeIntoBuffer(ableton::Link::Timeline timeline,
    double quantum,
    std::chrono::microseconds beginHostTime,
    std::size_t numSamples);

  ableton::Link& m_link;
  double m_sampleRate;
  Latency m_outputLatency;
  std::vector<double> m_bufferClock;
  std::vector<double> m_bufferReset;
  Data m_engineDataShared;
  Data m_engineDataLockFree;
  std::chrono::microseconds m_timeAtLastClick;
  std::chrono::microseconds m_timeAtLastReset;
  double m_phase;
  std::mutex m_engineDataGuard;
  double m_clockMultiplier;
};

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
