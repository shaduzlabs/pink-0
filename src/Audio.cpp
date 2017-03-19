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
const double k_sampleRate{44100.};
const std::size_t k_bufferSize{128};
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

#ifdef AUDIO_USE_RTAUDIO
  int numDevices = m_audioDevice.getDeviceCount();
#else
  int numDevices = Pa_GetDeviceCount();
#endif

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

#ifdef AUDIO_USE_RTAUDIO
int Audio::audioCallbackRTA(void* outputBuffer,
  void* inputBuffer,
  unsigned int nBufferFrames,
  double streamTime,
  RtAudioStreamStatus status,
  void* userData)
{

  using namespace std::chrono;
  float* buffer = static_cast<float*>(outputBuffer);
  Audio& platform = *static_cast<Audio*>(userData);
  Engine& engine = platform.m_engine;

  const auto hostTime = platform.m_hostTimeFilter.sampleTimeToHostTime(platform.m_sampleTime);

  platform.m_sampleTime += nBufferFrames;

  const auto bufferBeginAtOutput = hostTime + engine.outputLatency();

  engine.process(bufferBeginAtOutput, nBufferFrames);

  const auto& bufferReset = engine.bufferReset();
  const auto& bufferClock = engine.bufferClock();

  unsigned long i = 0;
  for (; i < nBufferFrames; ++i)
  {
    buffer[i * 2] = bufferReset[i];
    buffer[i * 2 + 1] = bufferClock[i];
  }

  return 0;
}

#else
// -------------------------------------------------------------------------------------------------

int Audio::audioCallbackPA(const void* /*inputBuffer*/,
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
#endif

// -------------------------------------------------------------------------------------------------

void Audio::initialize()
{
#ifdef AUDIO_USE_RTAUDIO

  if (m_audioDevice.getDeviceCount() < 1)
  {
    std::cerr << "No audio interfaces found" << std::endl;
    std::terminate();
  }

  try
  {
    m_audioStreamParameters.deviceId = m_audioDevice.getDefaultOutputDevice();
    m_audioStreamParameters.nChannels = 2;
    m_audioStreamParameters.firstChannel = 0;
    m_audioBufferSize = m_engine.bufferSize() / 2;

    RtAudio::DeviceInfo deviceInfo
      = m_audioDevice.getDeviceInfo(m_audioDevice.getDefaultOutputDevice());
    m_audioDevice.openStream(&m_audioStreamParameters,
      NULL,
      RTAUDIO_FLOAT32,
      static_cast<unsigned>(m_engine.sampleRate()),
      &m_audioBufferSize,
      &Audio::audioCallbackRTA,
      this);
  }
  catch (RtAudioError e)
  {
    std::cerr << "Could not initialize Audio Engine. " << e.getMessage() << std::endl;
    std::terminate();
  }
#else
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
    &audioCallbackPA,
    this);

  if (result)
  {
    throw std::runtime_error("Could not open stream.");
  }

  if (!m_stream)
  {
    throw std::runtime_error("No valid audio stream.");
  }
#endif
}

// -------------------------------------------------------------------------------------------------

void Audio::uninitialize()
{
#ifdef AUDIO_USE_RTAUDIO
  try
  {
    m_audioDevice.closeStream();
  }
  catch (RtAudioError e)
  {
    std::cerr << "Could not close Audio Stream. " << e.getMessage() << std::endl;
    std::terminate();
  }
#else
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
#endif
}

// -------------------------------------------------------------------------------------------------

void Audio::start()
{
#ifdef AUDIO_USE_RTAUDIO
  try
  {
    m_audioDevice.startStream();
  }
  catch (RtAudioError e)
  {
    std::cerr << "Could not start Audio Stream. " << e.getMessage() << std::endl;
    std::terminate();
  }
#else
  PaError result = Pa_StartStream(m_stream);
  if (result)
  {
    throw std::runtime_error("Could not start Audio Stream. (" + std::to_string(result) + ")");
  }
#endif
}

// -------------------------------------------------------------------------------------------------

void Audio::stop()
{
#ifdef AUDIO_USE_RTAUDIO
  try
  {
    m_audioDevice.stopStream();
  }
  catch (RtAudioError e)
  {
    std::cerr << "Could not start Audio Stream. " << e.getMessage() << std::endl;
    std::terminate();
  }
#else
  if (m_stream == nullptr)
  {
    return;
  }

  PaError result = Pa_StopStream(m_stream);
  if (result)
  {
    throw std::runtime_error("Could not stop Audio Stream. (" + std::to_string(result) + ")");
  }
#endif
}

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
