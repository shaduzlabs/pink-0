/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#include "Audio.h"

#include <chrono>
#include <stdexcept>

// -------------------------------------------------------------------------------------------------

namespace
{
const double k_sampleRate{11025.};
const std::size_t k_bufferSize{256};
}
// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

Audio::Audio(ableton::Link& link) : m_engine(link), m_sampleTime(0.)
{
  m_engine.setSampleRate(k_sampleRate);
  m_engine.setBufferSize(k_bufferSize);
  initialize();
  start();

  int numDevices;

  numDevices = Pa_GetDeviceCount();
  if (numDevices < 0)
  {
    throw std::runtime_error("ERROR: Pa_CountDevices returned " + std::to_string(numDevices));
  }
}

// -------------------------------------------------------------------------------------------------

Audio::~Audio()
{
  stop();
  uninitialize();
}

// -------------------------------------------------------------------------------------------------

int Audio::audioCallback(const void* /*inputBuffer*/,
  void* outputBuffer,
  unsigned long inNumFrames,
  const PaStreamCallbackTimeInfo* /*timeInfo*/,
  PaStreamCallbackFlags /*statusFlags*/,
  void* userData)
{
  using namespace std::chrono;
  float* buffer = static_cast<float*>(outputBuffer);
  Audio& platform = *static_cast<Audio*>(userData);
  Engine& engine = platform.m_engine;

  const auto hostTime = platform.m_hostTimeFilter.sampleTimeToHostTime(platform.m_sampleTime);

  platform.m_sampleTime += inNumFrames;

  const auto bufferBeginAtOutput = hostTime + engine.outputLatency();

  engine.process(bufferBeginAtOutput, inNumFrames);

  const auto& bufferReset = engine.bufferReset();
  const auto& bufferClock = engine.bufferClock();
  for (unsigned long i = 0; i < inNumFrames; ++i)
  {
    buffer[i * 2] = bufferReset[i];
    buffer[i * 2 + 1] = bufferClock[i];
  }

  return paContinue;
}

// -------------------------------------------------------------------------------------------------

void Audio::initialize()
{
  PaError result = Pa_Initialize();
  if (result)
  {
    throw std::runtime_error("Could not initialize Audio Engine (" + std::to_string(result) + ")");
  };

  PaStreamParameters outputParameters;
  outputParameters.device = Pa_GetDefaultOutputDevice();
  if (outputParameters.device == paNoDevice)
  {
    throw std::runtime_error("Could not get audio device");
  }

  outputParameters.channelCount = 2;
  outputParameters.sampleFormat = paFloat32;
  outputParameters.suggestedLatency
    = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;
  m_engine.setOutputLatency(
    std::chrono::microseconds(llround(outputParameters.suggestedLatency * 1.0e6)));
  result = Pa_OpenStream(&m_stream,
    NULL,
    &outputParameters,
    m_engine.sampleRate(),
    m_engine.bufferSize(),
    paClipOff,
    &audioCallback,
    this);

  if (result)
  {
    throw std::runtime_error("Could not open stream.");
  }

  if (!m_stream)
  {
    throw std::runtime_error("No valid audio stream.");
  }
}

// -------------------------------------------------------------------------------------------------

void Audio::uninitialize()
{
  PaError result = Pa_CloseStream(m_stream);
  if (result)
  {
    throw std::runtime_error("Could not close Audio Stream. (" + std::to_string(result) + ")");
  }
  Pa_Terminate();

  if (!m_stream)
  {
    throw std::runtime_error("No valid audio stream.");
  }
}

// -------------------------------------------------------------------------------------------------

void Audio::start()
{
  PaError result = Pa_StartStream(m_stream);
  if (result)
  {
    throw std::runtime_error("Could not start Audio Stream. (" + std::to_string(result) + ")");
  }
}

// -------------------------------------------------------------------------------------------------

void Audio::stop()
{
  if (m_stream == nullptr)
  {
    return;
  }

  PaError result = Pa_StopStream(m_stream);
  if (result)
  {
    throw std::runtime_error("Could not stop Audio Stream. (" + std::to_string(result) + ")");
  }
}

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
