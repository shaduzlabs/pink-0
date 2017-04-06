/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#include "Engine.h"

#include <cmath>

// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

using namespace ableton;

// -------------------------------------------------------------------------------------------------

Engine::Engine(Link& link)
  : m_link(link)
  , m_sampleRate(44100.)
  , m_outputLatency{}
  , m_engineDataShared({0., false, false, 4.})
  , m_engineDataLockFree(m_engineDataShared)
  , m_timeAtLastClick{}
  , m_timeAtLastReset{}
  , m_phase(0.)
  , m_clockMultiplier(1.0)
{
}

// -------------------------------------------------------------------------------------------------

void Engine::startPlaying()
{
  std::lock_guard<std::mutex> lock(m_engineDataGuard);
  m_engineDataShared.resetBeatTime = true;
  m_engineDataShared.isPlaying = true;
}

// -------------------------------------------------------------------------------------------------

void Engine::stopPlaying()
{
  std::lock_guard<std::mutex> lock(m_engineDataGuard);
  m_engineDataShared.isPlaying = false;
}

// -------------------------------------------------------------------------------------------------

bool Engine::isPlaying() const
{
  return m_engineDataShared.isPlaying;
}

// -------------------------------------------------------------------------------------------------

double Engine::beatTime() const
{
  const auto timeline = m_link.captureAppTimeline();
  return timeline.beatAtTime(m_link.clock().micros(), m_engineDataShared.quantum);
}

// -------------------------------------------------------------------------------------------------

std::size_t Engine::bufferSize() const
{
  return m_bufferClock.size();
}

// -------------------------------------------------------------------------------------------------

void Engine::setBufferSize(std::size_t size)
{
  m_bufferClock = std::vector<double>(size, 0.);
  m_bufferReset = std::vector<double>(size, 0.);
}

// -------------------------------------------------------------------------------------------------

double Engine::sampleRate() const
{
  return m_sampleRate;
}

// -------------------------------------------------------------------------------------------------

void Engine::setSampleRate(double sampleRate)
{
  m_sampleRate = sampleRate;
}

// -------------------------------------------------------------------------------------------------

double Engine::tempo() const
{
  return m_link.captureAppTimeline().tempo();
}

// -------------------------------------------------------------------------------------------------

void Engine::setTempo(double tempo)
{
  std::lock_guard<std::mutex> lock(m_engineDataGuard);
  m_engineDataShared.requestedTempo = tempo;
}

// -------------------------------------------------------------------------------------------------

double Engine::quantum() const
{
  return m_engineDataShared.quantum;
}

// -------------------------------------------------------------------------------------------------

void Engine::setQuantum(double quantum)
{
  std::lock_guard<std::mutex> lock(m_engineDataGuard);
  m_engineDataShared.quantum = quantum;
}

// -------------------------------------------------------------------------------------------------

double Engine::clockMultiplier() const
{
  return m_clockMultiplier;
}

// -------------------------------------------------------------------------------------------------

void Engine::setClockMultiplier(double cr_)
{
  m_clockMultiplier = cr_;
}

// -------------------------------------------------------------------------------------------------

const Engine::Latency& Engine::outputLatency() const
{
  return m_outputLatency;
}

// -------------------------------------------------------------------------------------------------

void Engine::setOutputLatency(Latency latency_)
{
  m_outputLatency = latency_;
}

// -------------------------------------------------------------------------------------------------

const Engine::AudioBuffer& Engine::bufferClock() const
{
  return m_bufferClock;
}

// -------------------------------------------------------------------------------------------------

const Engine::AudioBuffer& Engine::bufferReset() const
{
  return m_bufferReset;
}

// -------------------------------------------------------------------------------------------------

void Engine::process(const std::chrono::microseconds hostTime, const std::size_t numSamples)
{
  const auto engineData = pullEngineData();

  auto timeline = m_link.captureAudioTimeline();

  // Clear the buffer
  std::fill(m_bufferClock.begin(), m_bufferClock.end(), 1.);
  std::fill(m_bufferReset.begin(), m_bufferReset.end(), 1.);

  if (engineData.resetBeatTime)
  {
    // Reset the timeline so that beat 0 lands at the beginning of
    // this buffer and clear the flag.
    timeline.requestBeatAtTime(0, hostTime, engineData.quantum);
  }

  if (engineData.requestedTempo > 0)
  {
    // Set the newly requested tempo from the beginning of this buffer
    timeline.setTempo(engineData.requestedTempo, hostTime);
  }

  // Timeline modifications are complete, commit the results
  m_link.commitAudioTimeline(timeline);

  if (engineData.isPlaying)
  {
    // As long as the engine is playing, generate metronome clicks in
    // the buffer at the appropriate beats.
    renderMetronomeIntoBuffer(timeline, engineData.quantum, hostTime, numSamples);
  }
}

// -------------------------------------------------------------------------------------------------

Engine::Data Engine::pullEngineData()
{
  auto engineData = Data{};
  if (m_engineDataGuard.try_lock())
  {
    engineData.requestedTempo = m_engineDataShared.requestedTempo;
    m_engineDataShared.requestedTempo = 0;
    engineData.resetBeatTime = m_engineDataShared.resetBeatTime;
    m_engineDataShared.resetBeatTime = false;

    m_engineDataLockFree.isPlaying = m_engineDataShared.isPlaying;
    m_engineDataLockFree.quantum = m_engineDataShared.quantum;

    m_engineDataGuard.unlock();
  }
  engineData.isPlaying = m_engineDataLockFree.isPlaying;
  engineData.quantum = m_engineDataLockFree.quantum;

  return engineData;
}

// -------------------------------------------------------------------------------------------------

void Engine::renderMetronomeIntoBuffer(const Link::Timeline timeline,
  const double quantum,
  const std::chrono::microseconds beginHostTime,
  const std::size_t numSamples)
{
  using namespace std::chrono;

  // 1 ms click pulse duration, 20ms reset pulse duration
  static const auto resetDuration = duration<double>{0.02};

  // The number of microseconds that elapse between samples
  const auto microsPerSample = 1e6 / m_sampleRate;

  double pulseWrapFactor = 1.0 / (4.0 * m_clockMultiplier);

  for (std::size_t i = 0; i < numSamples; ++i)
  {
    double clockAmplitude = 1.;
    double resetAmplitude = 1.;

    // Compute the host time for this sample and the last.
    const auto hostTime = beginHostTime + microseconds(llround(i * microsPerSample));
    const auto lastSampleHostTime = hostTime - microseconds(llround(microsPerSample));

    // Only emit clock pulses for positive beat magnitudes. Negative beat
    // magnitudes are count-in beats.
    if (timeline.beatAtTime(hostTime, quantum) >= 0.)
    {
      auto clickDuration = duration<double>{(30.0 / timeline.tempo()) * pulseWrapFactor};

      if (timeline.phaseAtTime(hostTime, pulseWrapFactor)
          < timeline.phaseAtTime(lastSampleHostTime, pulseWrapFactor))
      {
        m_timeAtLastClick = hostTime;
      }

      const auto secondsAfterClick = duration_cast<duration<double>>(hostTime - m_timeAtLastClick);
      const auto secondsAfterReset = duration_cast<duration<double>>(hostTime - m_timeAtLastReset);

      auto phase = timeline.phaseAtTime(hostTime, quantum);
      if (phase < m_phase || phase == 0)
      {
        // reset!
        m_timeAtLastReset = hostTime;
      }
      m_phase = phase;

      if (secondsAfterClick < clickDuration)
      {
        clockAmplitude = -1.;
      }
      if (secondsAfterReset < resetDuration)
      {
        resetAmplitude = -1.;
      }
    }

    m_bufferClock[i] = clockAmplitude;
    m_bufferReset[i] = resetAmplitude;
  }
}

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
