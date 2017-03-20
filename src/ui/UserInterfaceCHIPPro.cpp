/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#include "ui/UserInterfaceCHIPPro.h"

#include <pink-config.h>

#include <sstream>

// -------------------------------------------------------------------------------------------------

namespace
{
}

// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

UserInterfaceCHIPPro::UserInterfaceCHIPPro(std::shared_ptr<Pink> pink_) : UserInterface(pink_)
{
}

// -------------------------------------------------------------------------------------------------

UserInterfaceCHIPPro::~UserInterfaceCHIPPro()
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceCHIPPro::statusChanged(bool enabled_)
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceCHIPPro::tempoChanged(double t_)
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceCHIPPro::loopLengthChanged(double l_)
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceCHIPPro::numPeersChanged(std::size_t n_)
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceCHIPPro::runStatusChanged(bool playing_)
{
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceCHIPPro::clockMultiplierChanged(const std::string& cm_)
{
}

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
