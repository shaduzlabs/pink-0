/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#pragma once

#include "ui/UserInterface.h"

#include <nmws/WebSocketServer.hpp>

#include <future>
#include <mutex>
#include <string>
#include <thread>


// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

class UserInterfaceWebSocket : public UserInterface
{
public:
  UserInterfaceWebSocket(std::shared_ptr<Pink> /*pink_*/);
  ~UserInterfaceWebSocket() override;

  void tempoChanged(double /*t_*/) override;
  void loopLengthChanged(double /*t_*/) override;

  void numPeersChanged(std::size_t /*n_*/) override;
  void runStatusChanged(bool /*playing_*/) override;

  void clockMultiplierChanged(const std::string& /*cm_*/) override;

private:
  void onMessageReceived(std::string /*message_*/);
  
  nmws::WebSocketServer m_wsServer;
};

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
