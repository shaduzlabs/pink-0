/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#include "ui/UserInterfaceWebSocket.h"

// -------------------------------------------------------------------------------------------------

namespace
{
const std::string k_wsAddressAndPort = "ws://127.0.0.1:8888";
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

void UserInterfaceWebSocket::tempoChanged(double t_)
{
  m_wsServer.send(std::to_string(t_));
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::loopLengthChanged(double l_)
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::numPeersChanged(std::size_t n_)
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::runStatusChanged(bool playing_)
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::clockMultiplierChanged(const std::string& cm_)
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceWebSocket::onMessageReceived(std::string message_)
{
 // \todo: process message
}

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
