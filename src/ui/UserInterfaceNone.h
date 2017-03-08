/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      #############################################################
shaduzlabs.com #####*/

#pragma once

#include <future>
#include <mutex>
#include <string>
#include <thread>

#include "UserInterface.h"

// -------------------------------------------------------------------------------------------------

namespace sl {
namespace pi {

// -------------------------------------------------------------------------------------------------

class UserInterfaceNone : public UserInterface {
public:
  UserInterfaceNone(std::shared_ptr<Pink> pink_) : UserInterface(pink_) {
    if (pink_) {
      pink_->togglePlay();
    }
  }

  void tempoChanged(double /*t_*/) override {}
  void loopLengthChanged(double /*t_*/) override {}
  void numPeersChanged(std::size_t /*n_*/) override {}
  void runStatusChanged(bool /*playing_*/) override {}
  void clockMultiplierChanged(const std::string & /*cm_*/) override {}
};

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
