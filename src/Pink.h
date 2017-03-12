/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#pragma once

#include <memory>
#include <mutex>

#include <ableton/Link.hpp>

#include "Audio.h"

// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

class Pink
{
public:
  struct ClockMultiplier
  {
    ClockMultiplier(double v, std::string l) : value(v), label(std::move(l))
    {
    }

    double value;
    std::string label;
  };

  using ClockMultipliers = std::vector<ClockMultiplier>;

  Pink(double /*tempo_*/, double /*length_*/, double /*clockMultiplier_*/);

  bool isEnabled() const;
  void setEnabled(bool /*enabled_*/);

  void togglePlay();
  bool isPlaying();

  double tempo();
  void setTempo(double /*tempo_*/);

  double loopLength();
  void setLoopLength(double /*loopLength_*/);

  std::size_t numClockMultipliers() const;
  std::size_t clockMultiplierIndex() const;
  const std::string& clockMultiplierLabel() const;
  const std::string& clockMultiplierLabel(std::size_t /*index_*/) const;
  void setClockMultiplierIndex(std::size_t /*index_*/);

  std::size_t numPeers() const;

  class Listener
  {
  public:
    using Id = std::string;

    Listener();
    virtual ~Listener() = default;
    const Id& pinkListenerId() const
    {
      return m_id;
    }
    virtual void statusChanged(bool /*enabled_*/) = 0;
    virtual void tempoChanged(double /*t_*/) = 0;
    virtual void loopLengthChanged(double /*l_*/) = 0;
    virtual void numPeersChanged(std::size_t /*n_*/) = 0;
    virtual void runStatusChanged(bool /*playing_*/) = 0;
    virtual void clockMultiplierChanged(const std::string& /*cm_*/) = 0;

  private:
    Id m_id;
  };

  using Listeners = std::map<Listener::Id, Listener*>;

  void addListener(const Listener::Id /*id_*/, Listener* /*l_*/);
  void removeListener(const Listener::Id /*id_*/);

private:
  void setNumPeers(std::size_t numPeers_);

  unsigned m_defaultClockMultiplier;
  unsigned m_currentClockMultiplier;

  std::mutex m_mtxEnabled;
  std::mutex m_mtxRunning;

  ableton::Link m_link;
  Audio m_audio;

  Listeners m_listeners;
};

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
