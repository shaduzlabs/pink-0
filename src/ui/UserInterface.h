/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#pragma once

#include <atomic>
#include <functional>

#include "../Pink.h"

// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

class UserInterface : public Pink::Listener
{
public:
  using tCallbackEncoderChanged = std::function<void(bool, bool)>;
  using tCallbackButtonPressed = std::function<void()>;

  UserInterface(std::shared_ptr<Pink> pink_) : m_pink(pink_)
  {
    m_pink->addListener(pinkListenerId(), this);
  }

  ~UserInterface() override
  {
    m_pink->removeListener(pinkListenerId());
  }

protected:
  void onEncoderChanged(tCallbackEncoderChanged fnEncoderChanged_)
  {
    m_cbEncoderChanged = std::move(fnEncoderChanged_);
  }

  void onRunButtonPressed(tCallbackButtonPressed fnRunButtonPressed_)
  {
    m_cbRunButtonPressed = std::move(fnRunButtonPressed_);
  }

  void onEncoderButtonPressed(tCallbackButtonPressed fnEncoderButtonPressed_)
  {
    m_cbEncoderButtonPressed = std::move(fnEncoderButtonPressed_);
  }

  void encoderChanged(bool direction_, bool buttonPressed_)
  {
    if (m_cbEncoderChanged)
    {
      m_cbEncoderChanged(direction_, buttonPressed_);
    }
  }

  void runButtonPressed()
  {
    if (m_cbRunButtonPressed)
    {
      m_cbRunButtonPressed();
    }
  }

  void encoderButtonPressed()
  {
    if (m_cbEncoderButtonPressed)
    {
      m_cbEncoderButtonPressed();
    }
  }

  std::shared_ptr<Pink> m_pink;


private:
  tCallbackEncoderChanged m_cbEncoderChanged;
  tCallbackButtonPressed m_cbRunButtonPressed;
  tCallbackButtonPressed m_cbEncoderButtonPressed;
};

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
