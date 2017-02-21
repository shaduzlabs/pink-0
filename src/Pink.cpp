/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#include "Pink.h"

// -------------------------------------------------------------------------------------------------

namespace
{

// -------------------------------------------------------------------------------------------------

static sl::pi::Pink::ClockMultipliers k_clockMultipliers{{1 / 64., "1/64"},
  {1 / 32., "1/32"},
  {1 / 16., "1/16"},
  {1 / 8., "1/8"},
  {1 / 4., "1/4"},
  {1 / 3., "1/3"},
  {1 / 2., "1/2"},
  {1., "1"},
  {2., "2"},
  {3., "3"},
  {4., "4"},
  {5., "5"},
  {6., "6"},
  {7., "7"},
  {8., "8"},
  {9., "9"},
  {10., "10"},
  {11., "11"},
  {12., "12"},
  {13., "13"},
  {14., "14"},
  {15., "15"},
  {16., "16"},
  {24., "24"},
  {32., "32"},
  {48., "48"},
  {64., "64"},
  {128., "128"}};

// -------------------------------------------------------------------------------------------------

const std::size_t kListenerIdLength{16};

// -------------------------------------------------------------------------------------------------

std::string randomId(std::size_t length_)
{
  static const char characters[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";

  std::string id;

  for (int i = 0; i < length_; ++i)
  {
    id += characters[rand() % (sizeof(characters) - 1)];
  }

  return id;
}

// -------------------------------------------------------------------------------------------------

} // namespace

// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

Pink::Pink(double tempo_, double length_, double clockMultiplier_) : m_link(tempo_), m_audio(m_link)
{
  for (unsigned i = 0; i < k_clockMultipliers.size(); ++i)
  {
    if (k_clockMultipliers[i].value == 1.)
    {
      m_defaultClockMultiplier = i;
    }
  }
  m_currentClockMultiplier = m_defaultClockMultiplier;

  if (clockMultiplier_ > 0.)
  {
    for (unsigned i = 0; i < k_clockMultipliers.size(); ++i)
    {
      if (k_clockMultipliers[i].value == clockMultiplier_)
      {
        m_currentClockMultiplier = i;
      }
    }
  }
  using namespace std::placeholders;

  m_link.setTempoCallback(std::bind(&Pink::setTempo, this, _1));

  m_link.setNumPeersCallback(std::bind(&Pink::setNumPeers, this, _1));

  m_link.enable(true);

  setTempo(tempo_);
  setLoopLength(length_);

  m_audio.engine().setClockMultiplier(k_clockMultipliers[m_currentClockMultiplier].value);
}

// -------------------------------------------------------------------------------------------------

void Pink::togglePlay()
{
  std::lock_guard<std::mutex> lock(m_mtxRunning);
  if (m_audio.engine().isPlaying())
  {
    m_audio.engine().stopPlaying();
  }
  else
  {
    m_audio.engine().startPlaying();
  }

  for (const auto& l : m_listeners)
  {
    auto listener = l.second;
    if (listener == nullptr)
    {
      continue;
    }
    listener->runStatusChanged(m_audio.engine().isPlaying());
  }
}

// -------------------------------------------------------------------------------------------------

bool Pink::isPlaying()
{
  return m_audio.engine().isPlaying();
}

// -------------------------------------------------------------------------------------------------

double Pink::tempo()
{
  return m_audio.engine().tempo();
}

// -------------------------------------------------------------------------------------------------

void Pink::setTempo(double tempo_)
{
  m_audio.engine().setTempo(tempo_);

  for (const auto& l : m_listeners)
  {
    auto listener = l.second;
    if (listener == nullptr)
    {
      continue;
    }
    listener->tempoChanged(tempo_);
  }
}

// -------------------------------------------------------------------------------------------------

void Pink::setNumPeers(std::size_t numPeers_)
{
  for (const auto& l : m_listeners)
  {
    auto listener = l.second;
    if (listener == nullptr)
    {
      continue;
    }
    listener->numPeersChanged(numPeers_);
  }
}

// -------------------------------------------------------------------------------------------------

double Pink::loopLength()
{
  return m_audio.engine().quantum();
}

// -------------------------------------------------------------------------------------------------

void Pink::setLoopLength(double length_)
{
  m_audio.engine().setQuantum(length_);

  for (const auto& l : m_listeners)
  {
    auto listener = l.second;
    if (listener == nullptr)
    {
      continue;
    }
    listener->loopLengthChanged(length_);
  }
}

// -------------------------------------------------------------------------------------------------

std::size_t Pink::numClockMultipliers() const
{
  return k_clockMultipliers.size();
}

// -------------------------------------------------------------------------------------------------

std::size_t Pink::clockMultiplierIndex() const
{
  return m_currentClockMultiplier;
}

// -------------------------------------------------------------------------------------------------

const std::string& Pink::clockMultiplierLabel() const
{
  return k_clockMultipliers[m_currentClockMultiplier].label;
}

// -------------------------------------------------------------------------------------------------

void Pink::setClockMultiplierIndex(std::size_t index_)
{
  if (index_ < k_clockMultipliers.size())
  {
    m_currentClockMultiplier = index_;
    m_audio.engine().setClockMultiplier(k_clockMultipliers[m_currentClockMultiplier].value);
    for (const auto& l : m_listeners)
    {
      auto listener = l.second;
      if (listener == nullptr)
      {
        continue;
      }
      listener->clockMultiplierChanged(k_clockMultipliers[m_currentClockMultiplier].label);
    }
  }
}

// -------------------------------------------------------------------------------------------------

Pink::Listener::Listener() : m_id(randomId(kListenerIdLength))
{
}

// -------------------------------------------------------------------------------------------------

void Pink::addListener(const Listener::Id id_, Listener* l_)
{
  m_listeners[id_] = l_;
}

// -------------------------------------------------------------------------------------------------

void Pink::removeListener(const Listener::Id id_)
{
  m_listeners.erase(id_);
}

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
