/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#include "ui/UserInterfaceWebSocket.h"

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include <sstream>

// -------------------------------------------------------------------------------------------------

namespace
{
const std::string k_wsAddressAndPort = "ws://127.0.0.1:8888";

struct Command
{
  enum class Type : uint8_t
  {
    togglePlay,
    isEnabled,
    isPlaying,
    getTempo,
    getLoopLength,
    getMultiplierIndex,
    getMultipliers,
    getNumberOfPeers,
    setEnabled,
    setTempo,
    setLoopLength,
    setMultiplierIndex,
    unknown,
  };

  Command() : argument(0.)
  {
  }

  template <typename Archive>
  void serialize(Archive& ar)
  {
    ar(CEREAL_NVP(command), CEREAL_NVP(argument));
  }

  Type type() const
  {
    if (command == "togglePlay")
    {
      return Type::togglePlay;
    }
    if (command == "isEnabled")
    {
      return Type::isEnabled;
    }
    if (command == "isPlaying")
    {
      return Type::isPlaying;
    }
    else if (command == "getTempo")
    {
      return Type::getTempo;
    }
    else if (command == "getLoopLength")
    {
      return Type::getLoopLength;
    }
    else if (command == "getMultiplierIndex")
    {
      return Type::getMultiplierIndex;
    }
    else if (command == "getMultipliers")
    {
      return Type::getMultipliers;
    }
    else if (command == "getNumberOfPeers")
    {
      return Type::getNumberOfPeers;
    }
    else if (command == "setEnabled")
    {
      return Type::setEnabled;
    }
    else if (command == "setTempo")
    {
      return Type::setTempo;
    }
    else if (command == "setLoopLength")
    {
      return Type::setLoopLength;
    }
    else if (command == "setMultiplierIndex")
    {
      return Type::setMultiplierIndex;
    }
    return Type::unknown;
  }

  std::string command;
  double argument;
};
}

// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

UserInterfaceWebSocket::UserInterfaceWebSocket(std::shared_ptr<Pink> pink_) : UserInterface(pink_)
{
  using namespace std::placeholders;

  m_wsServer.bind(k_wsAddressAndPort);
  m_wsServer.setMessageCallback(std::bind(&UserInterfaceWebSocket::onMessageReceived, this, _1));
  m_wsServer.start();
}

// -------------------------------------------------------------------------------------------------

UserInterfaceWebSocket::~UserInterfaceWebSocket()
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::statusChanged(bool enabled_)
{
  std::ostringstream ss;
  {
    cereal::JSONOutputArchive archive(ss);
    auto enabled = enabled_;
    archive(CEREAL_NVP(enabled));
  }
  m_wsServer.send(ss.str());
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::tempoChanged(double t_)
{
  std::ostringstream ss;
  {
    cereal::JSONOutputArchive archive(ss);
    auto tempo = m_pink->tempo();
    archive(CEREAL_NVP(tempo));
  }
  m_wsServer.send(ss.str());
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::loopLengthChanged(double l_)
{
  std::ostringstream ss;

  {
    cereal::JSONOutputArchive archive(ss);
    auto loopLength = m_pink->loopLength();
    archive(CEREAL_NVP(loopLength));
  }
  m_wsServer.send(ss.str());
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::numPeersChanged(std::size_t n_)
{
  std::ostringstream ss;
  {
    cereal::JSONOutputArchive archive(ss);
    auto numPeers = m_pink->numPeers();
    archive(CEREAL_NVP(numPeers));
  }
  m_wsServer.send(ss.str());
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::runStatusChanged(bool playing_)
{
  std::ostringstream ss;
  {
    cereal::JSONOutputArchive archive(ss);
    auto playing = playing_;
    archive(CEREAL_NVP(playing));
  }
  m_wsServer.send(ss.str());
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::clockMultiplierChanged(const std::string& cm_)
{
  std::ostringstream ss;
  {
    cereal::JSONOutputArchive archive(ss);
    auto multiplierIndex = m_pink->clockMultiplierIndex();
    archive(CEREAL_NVP(multiplierIndex));
  }
  m_wsServer.send(ss.str());
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::onMessageReceived(std::string message_)
{
  Command command;
  std::ostringstream ss;

  try
  {

    std::istringstream inputStream(message_);
    {
      cereal::JSONInputArchive archive(inputStream);
      archive(command);
    }
  }
  catch (const cereal::RapidJSONException& e)
  {
    std::cerr << "Error: " << message_ << std::endl;
    return;
  }

  switch (command.type())
  {
    case Command::Type::togglePlay:
    {
      m_pink->togglePlay();
      break;
    }
    case Command::Type::isEnabled:
    {
      {
        cereal::JSONOutputArchive archive(ss);
        auto enabled = m_pink->isEnabled();
        archive(CEREAL_NVP(enabled));
      }
      m_wsServer.send(ss.str());
      break;
    }
    case Command::Type::isPlaying:
    {
      {
        cereal::JSONOutputArchive archive(ss);
        auto playing = m_pink->isPlaying();
        archive(CEREAL_NVP(playing));
      }
      m_wsServer.send(ss.str());
      break;
    }
    case Command::Type::getTempo:
    {
      {
        cereal::JSONOutputArchive archive(ss);
        auto tempo = m_pink->tempo();
        archive(CEREAL_NVP(tempo));
      }
      m_wsServer.send(ss.str());
      break;
    }
    case Command::Type::getLoopLength:
    {
      {
        cereal::JSONOutputArchive archive(ss);
        auto loopLength = m_pink->loopLength();
        archive(CEREAL_NVP(loopLength));
      }
      m_wsServer.send(ss.str());
      break;
    }
    case Command::Type::getMultipliers:
    {
      {
        cereal::JSONOutputArchive archive(ss);
        std::vector<std::string> multipliers;
        for (std::size_t i = 0; i < m_pink->numClockMultipliers(); ++i)
        {
          multipliers.push_back(m_pink->clockMultiplierLabel(i));
        }
        archive(CEREAL_NVP(multipliers));
      }
      m_wsServer.send(ss.str());
      break;
    }
    case Command::Type::getMultiplierIndex:
    {
      {
        cereal::JSONOutputArchive archive(ss);
        auto multiplierIndex = m_pink->clockMultiplierIndex();
        archive(CEREAL_NVP(multiplierIndex));
      }
      m_wsServer.send(ss.str());
      break;
    }
    case Command::Type::getNumberOfPeers:
    {
      {
        cereal::JSONOutputArchive archive(ss);
        auto numPeers = m_pink->numPeers();
        archive(CEREAL_NVP(numPeers));
      }
      m_wsServer.send(ss.str());
      break;
    }
    case Command::Type::setEnabled:
    {
      m_pink->setEnabled(command.argument > 0);
      break;
    }
    case Command::Type::setTempo:
    {
      m_pink->setTempo(command.argument);
      break;
    }
    case Command::Type::setLoopLength:
    {
      m_pink->setLoopLength(command.argument);
      break;
    }
    case Command::Type::setMultiplierIndex:
    {
      m_pink->setClockMultiplierIndex(static_cast<std::size_t>(command.argument));
      break;
    }
    default:
    {
      return;
    }
  }
}

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
