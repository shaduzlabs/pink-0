/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#include "ui/UserInterfacePiZero.h"

#include <cmath>
#include <sstream>

#ifdef __arm__
#include <wiringPi.h>
#else
#include <iostream>
#endif

// -------------------------------------------------------------------------------------------------

namespace
{

const unsigned kPinLedTempo{15};   // BCM 14
const unsigned kPinLedLength{9};   // BCM 3
const unsigned kPinLedPPQN{8};     // BCM 2
const unsigned kPinLedRunning{11}; // BCM 7

const unsigned kPinEncoderA{24};   // BCM 19
const unsigned kPinEncoderB{27};   // BCM 16
const unsigned kPinBtnEncoder{22}; // BCM 6
const unsigned kPinBtnRun{21};     // BCM 5

const unsigned kDisplayRefreshPeriodUs{500};
unsigned kDisplayNumSegments{8};
unsigned kDisplayNumDigits{4};

const unsigned kPinDisplaySegments[] = {10 /*BCM8*/,
  6 /*BCM25*/,
  12 /*BCM10*/,
  0 /*BCM17*/,
  13 /*BCM9*/,
  4 /*BCM23*/,
  7 /*BCM4*/,
  2 /*BCM27*/};
const unsigned kPinDisplayDigits[] = {14 /*BCM11*/, 5 /*BCM24*/, 3 /*BCM22*/, 16 /*BCM15*/};

const unsigned kPinPWM1{1};  // BCM 18
const unsigned kPinPWM2{23}; // BCM 13

const uint8_t kDisplayFontData[] = {
#include "Font7Seg.h"
};

// -------------------------------------------------------------------------------------------------

#ifndef __arm__

#define LOW 0
#define HIGH 1
#define INPUT 0
#define OUTPUT 1
#define PUD_OFF 0
#define PUD_DOWN 1
#define PUD_UP 2
#define INT_EDGE_SETUP 0
#define INT_EDGE_FALLING 1
#define INT_EDGE_RISING 2
#define INT_EDGE_BOTH 3

void wiringPiSetup()
{
  std::cout << "wirintPiSetup()" << std::endl;
}

// -------------------------------------------------------------------------------------------------

void digitalWrite(int pin_, int value_)
{
  std::cout << "Pin #" << pin_ << " set to " << (value_ > 0 ? "HIGH" : "LOW") << std::endl;
}

// -------------------------------------------------------------------------------------------------

void pinMode(int pin_, int mode_)
{
  std::cout << "Pin #" << pin_ << " configured as " << (mode_ > 0 ? "OUTPUT" : "INPUT")
            << std::endl;
}

// -------------------------------------------------------------------------------------------------

void pinModeAlt(int pin_, int alt_)
{
  std::cout << "Pin #" << pin_ << " configured as alt function " << alt_ << std::endl;
}

// -------------------------------------------------------------------------------------------------

void pullUpDnControl(int pin_, int cfg_)
{
  if (cfg_ > 0)
  {
    std::cout << "Pin #" << pin_ << " pull-" << (cfg_ == PUD_UP ? "up" : "down") << " enabled"
              << std::endl;
  }
}

// -------------------------------------------------------------------------------------------------

int digitalRead(int pin_)
{
  std::cout << "Reading from Pin #" << pin_ << std::endl;
  return 0;
}

// -------------------------------------------------------------------------------------------------

void wiringPiISR(int pin_, int mode_, void (*fn_)(void))
{
  std::cout << "Pin #" << pin_ << " interrupt mode " << mode_ << std::endl;
}

// -------------------------------------------------------------------------------------------------

#endif

inline void writeToDisplay(unsigned digit_, char c_, bool dot_)
{
  for (unsigned i = 0; i < kDisplayNumDigits; i++)
  {
    digitalWrite(kPinDisplayDigits[i], (i == digit_ ? LOW : HIGH));
  }

  uint8_t charNum = static_cast<uint8_t>(c_);
  if ((charNum != 32 && charNum < 45) || charNum > 90)
  {
    return;
  }

  if (charNum == 32)
  {
    for (unsigned i = 0; i < kDisplayNumSegments; i++)
    {
      digitalWrite(kPinDisplaySegments[i], LOW);
    }
  }
  else
  {
    auto charData = kDisplayFontData[charNum - 45] | (dot_ ? 0b10000000 : 0);
    for (unsigned i = 0; i < kDisplayNumSegments; i++)
    {
      digitalWrite(kPinDisplaySegments[i], ((charData & (1 << i)) > 0) ? HIGH : LOW);
    }
  }
}

// -------------------------------------------------------------------------------------------------
}

// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

// wiringPi only allows to set an ISR handler through a void pointer, a proper solution would be to
// fork the repository at https://git.drogon.net/?p=wiringPi and change wiringPiISR so that it could
// take a std::function as third argument, but for the time being we'll live with the horrible
// global variable below. Don't you dare creating more than one instance of this class (would that
// make any sense, BTW?)
UserInterfacePiZero* pInstance;

// -------------------------------------------------------------------------------------------------

UserInterfacePiZero::UserInterfacePiZero(std::shared_ptr<Pink> pink_)
  : UserInterface(pink_)
  , m_threadDisplay(&UserInterfacePiZero::hardwareIO, this)
  , m_displayState(DisplayState::tempo)
  , m_lastEncoderValue(0)
{
  pInstance = this;

  wiringPiSetup();

  // setup leds
  pinMode(kPinLedTempo, OUTPUT);
  pinMode(kPinLedLength, OUTPUT);
  pinMode(kPinLedPPQN, OUTPUT);
  pinMode(kPinLedRunning, OUTPUT);

  // setup encoder
  pinMode(kPinEncoderA, INPUT);
  pinMode(kPinEncoderB, INPUT);
  pinMode(kPinBtnEncoder, INPUT);
  pullUpDnControl(kPinEncoderA, PUD_UP);
  pullUpDnControl(kPinEncoderB, PUD_UP);
  pullUpDnControl(kPinBtnEncoder, PUD_UP);

  wiringPiISR(kPinEncoderA, INT_EDGE_BOTH, encoderHandler);
  wiringPiISR(kPinEncoderB, INT_EDGE_BOTH, encoderHandler);
  wiringPiISR(kPinBtnEncoder, INT_EDGE_BOTH, encoderButtonHandler);

  // setup "run" button
  pullUpDnControl(kPinBtnRun, PUD_UP);
  pinMode(kPinBtnRun, INPUT);
  wiringPiISR(kPinBtnRun, INT_EDGE_BOTH, runButtonHandler);

  // setup display
  for (unsigned i = 0; i < kDisplayNumSegments; i++)
  {
    pinMode(kPinDisplaySegments[i], OUTPUT);
  }
  for (unsigned i = 0; i < kDisplayNumDigits; i++)
  {
    pinMode(kPinDisplayDigits[i], OUTPUT);
  }

  // setup pwm pins (clock & reset)
  pinModeAlt(kPinPWM1, 2); // ALT5
  pinModeAlt(kPinPWM2, 4); // ALT0

  m_engineTempo = m_pink->tempo();
  m_engineLoopLength = m_pink->loopLength();
  m_engineClockMultiplier = m_pink->clockMultiplierLabel();

  registerCallbacks();
}

// -------------------------------------------------------------------------------------------------

UserInterfacePiZero::~UserInterfacePiZero()
{
  m_running.store(false);
  if (m_threadDisplay.joinable())
  {
    m_threadDisplay.join();
  }
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::tempoChanged(double t_)
{
  m_engineTempo = t_;
  m_updateUI.store(true);
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::loopLengthChanged(double l_)
{
  m_engineLoopLength = l_;
  m_updateUI.store(true);
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::numPeersChanged(std::size_t n_)
{
  m_numPeers = n_;
  m_updateUI.store(true);
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::runStatusChanged(bool playing_)
{
  m_engineRunning = playing_;
  m_updateUI.store(true);
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::clockMultiplierChanged(const std::string& cm_)
{
  m_engineClockMultiplier = cm_;
  m_updateUI.store(true);
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::display(int value_)
{
  std::string strValue = std::to_string(static_cast<int>(value_));
  std::string strFill = "";
  bool showDot = false;
  if (strValue.length() < 4)
  {
    strFill = std::string(4 - strValue.length(), ' ');
  }
  else if (strValue.length() > 4)
  {
    strValue = "-HI-";
  }

  {
    std::lock_guard<std::mutex> lock(m_mtxDisplay);
    m_displayString = strFill + strValue;
    m_displayShowDot = showDot;
  }
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::display(double value_)
{
  double integral;
  double fractional = modf(value_, &integral);
  std::string strValue = std::to_string(static_cast<int>(integral));
  std::string strFill = "";
  bool showDot = false;
  if (strValue.length() < 4)
  {
    auto uFractional = lround(fractional * 10);
    if (uFractional > 9)
    {
      uFractional = 0;
      strValue = std::to_string(static_cast<int>(integral + 1));
    }
    std::string strFractional = std::to_string(uFractional);
    unsigned length = strValue.length() + strFractional.length();
    unsigned leftFills = length > 4 ? 0 : 4 - length;
    strFill = std::string(leftFills, ' ');
    strValue.append(strFractional);
    showDot = true;
  }
  else if (strValue.length() > 4)
  {
    strValue = "-HI-";
  }

  {
    std::lock_guard<std::mutex> lock(m_mtxDisplay);
    m_displayString = strFill + strValue;
    m_displayShowDot = showDot;
  }
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::display(const std::string& value_)
{
  std::string strValue(value_);
  std::string strFill = "";
  if (strValue.length() < 4)
  {
    auto length = strValue.length();
    auto leftFills = length > 4 ? 0 : 4 - length;
    strFill = std::string(leftFills, ' ');
  }
  else
  {
    strValue = strValue.substr(0, 4);
  }
  {
    std::lock_guard<std::mutex> lock(m_mtxDisplay);
    m_displayString = strFill + strValue;
    m_displayShowDot = false;
  }
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::registerCallbacks()
{
  onEncoderChanged([this](bool clockwise_, bool buttonPressed_) {


    switch (m_displayState)
    {
      case DisplayState::tempo:
      {
        auto tempo = m_pink->tempo();
        float step = 0.25 * (clockwise_ ? 1. : -1.);
        tempo += step;
        m_pink->setTempo(tempo);
        break;
      }

      case DisplayState::length:
      {
        float step = 0.5 * (clockwise_ ? 1. : -1.);
        auto loopLength = m_pink->loopLength() + step;
        if (loopLength > 0.)
        {
          m_pink->setLoopLength(loopLength);
        }
        break;
      }
      case DisplayState::ppqn:
      {
        auto currentMultiplier = m_pink->clockMultiplierIndex();
        auto numMultipliers = m_pink->numClockMultipliers();
        if (clockwise_)
        {
          if ((currentMultiplier + 1) < numMultipliers)
          {
            m_pink->setClockMultiplierIndex(++currentMultiplier);
          }
        }
        else
        {
          if (currentMultiplier >= 1)
          {
            m_pink->setClockMultiplierIndex(--currentMultiplier);
          }
        }
        break;
      }
      default:
        break;
    }

  });

  onRunButtonPressed([this]() { m_pink->togglePlay(); });

  onEncoderButtonPressed([this]() {

    if (m_displayState == DisplayState::tempo)
    {
      m_displayState = DisplayState::length;
    }
    else if (m_displayState == DisplayState::length)
    {
      m_displayState = DisplayState::ppqn;
    }
    else if (m_displayState == DisplayState::ppqn)
    {
      m_displayState = DisplayState::peers;
    }
    else if (m_displayState == DisplayState::peers)
    {
      m_displayState = DisplayState::tempo;
    }
    m_updateUI.store(true);

  });
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::hardwareIO()
{
  unsigned currentDigit = 0;

  while (m_running)
  {
    bool expected = true;
    if (!m_updateUI.compare_exchange_strong(expected, false))
    {
      updateUI();
    }
    std::string displayString;
    {
      std::lock_guard<std::mutex> lock(m_mtxDisplay);
      displayString = m_displayString;
    }

    writeToDisplay(
      currentDigit, m_displayString[currentDigit], m_displayShowDot && currentDigit == 2);
    currentDigit = (currentDigit + 1) % kDisplayNumDigits;


    std::this_thread::sleep_for(std::chrono::microseconds(kDisplayRefreshPeriodUs));
  }
}

void UserInterfacePiZero::updateUI()
{
  setLed(Led::running, m_engineRunning);

  switch (m_displayState)
  {
    case DisplayState::tempo:
    {
      setLed(Led::tempo, true);
      setLed(Led::length, false);
      setLed(Led::ppqn, false);
      display(m_engineTempo);
      break;
    }
    case DisplayState::length:
    {
      setLed(Led::tempo, false);
      setLed(Led::length, true);
      setLed(Led::ppqn, false);
      display(m_engineLoopLength);
      break;
    }
    case DisplayState::ppqn:
    {
      setLed(Led::tempo, false);
      setLed(Led::length, false);
      setLed(Led::ppqn, true);

      display(m_engineClockMultiplier);
      break;
    }
    case DisplayState::peers:
    {
      setLed(Led::tempo, false);
      setLed(Led::length, false);
      setLed(Led::ppqn, false);
      std::stringstream ss;
      ss << m_numPeers << "P";
      display(ss.str());
      break;
    }
    default:
      break;
  }
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::setLed(Led led_, bool state_)
{
  int value = state_ ? HIGH : LOW;
  switch (led_)
  {
    case Led::tempo:
    {
      digitalWrite(kPinLedTempo, value);
      break;
    }
    case Led::length:
    {
      digitalWrite(kPinLedLength, value);
      break;
    }
    case Led::ppqn:
    {
      digitalWrite(kPinLedPPQN, value);
      break;
    }
    case Led::running:
    {
      digitalWrite(kPinLedRunning, value);
      break;
    }
    default:
    {
      break;
    }
  }
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::encoderHandler()
{
  int msb = digitalRead(kPinEncoderA);
  int lsb = digitalRead(kPinEncoderB);
  int encoded = (msb << 1) | lsb;

  if (encoded == 0b00)
  {
    if (pInstance->m_lastEncoderValue == 0b01)
    {
      pInstance->encoderChanged(true, pInstance->m_encoderButtonPressed);
    }
    else if (pInstance->m_lastEncoderValue == 0b10)
    {
      pInstance->encoderChanged(false, pInstance->m_encoderButtonPressed);
    }
  }

  pInstance->m_lastEncoderValue = encoded;
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::runButtonHandler()
{
  bool pressed = digitalRead(kPinBtnRun) == HIGH;
  bool expected = !pressed;
  if (!pInstance->m_runButtonPressed.compare_exchange_strong(expected, pressed))
  {
    return;
  }
  auto debounce = [pressed]() {

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    if ((digitalRead(kPinBtnRun) == HIGH) == pressed)
    {
      if (pressed)
      {
        pInstance->runButtonPressed();
      }

      pInstance->m_runButtonPressed = pressed;
    }
  };

  pInstance->m_runButtonDebouncer = std::async(std::launch::async, debounce);
}

// -------------------------------------------------------------------------------------------------

void UserInterfacePiZero::encoderButtonHandler()
{
  bool pressed = digitalRead(kPinBtnEncoder) == HIGH;
  bool expected = !pressed;
  if (!pInstance->m_encoderButtonPressed.compare_exchange_strong(expected, pressed))
  {
    return;
  }
  auto debounce = [pressed]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    if ((digitalRead(kPinBtnEncoder) == HIGH) == pressed)
    {
      if (pressed)
      {
        pInstance->encoderButtonPressed();
      }
      pInstance->m_encoderButtonPressed = pressed;
    }
  };

  pInstance->m_encoderButtonDebouncer = std::async(std::launch::async, debounce);
}

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
