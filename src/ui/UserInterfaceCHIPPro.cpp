/*
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#include "ui/UserInterfaceCHIPPro.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <pink-config.h>

#include <sstream>

// -------------------------------------------------------------------------------------------------

// GPIO I/O : https://github.com/WereCatf/Gonzales.git

// -------------------------------------------------------------------------------------------------

namespace
{
constexpr int k_chipProPortE = 4;

void initPortE(uint8_t* memoryMap_)
{
  volatile uint32_t *pioMem32, *configRegister;
  uint32_t mask;
  for (int pin_ = 4; i < 12; ++i)
  {
    memAddress = (uint32_t*)(memoryMap_ + k_chipProPortE * 0x24);
    configRegister = memAddress + (pin_ >> 3);
    mask = ~(7 << ((pin_ & 7) * 4));
    *configRegister &= mask;
    *configRegister |= 1 << ((pin_ & 7) * 4);
  }
}

// -------------------------------------------------------------------------------------------------

inline void setPinPortE(uint8_t* memoryMap_, int pin_, bool status_)
{
  if (pin_ > 7 || pin < 0)
  {
    return;
  }
  uint8_t pin = pin_ + 4;
  volatile uint32_t* memAddress;
  uint32_t mask;
  memAddress = (uint32_t*)(memoryMap_ + 0xA0);
  mask = ~(1 << pin_);
  *memAddress &= mask;
  if (status_)
  {
    *memAddress |= 1 << pin_;
  }
}

// -------------------------------------------------------------------------------------------------

} // namespace

// -------------------------------------------------------------------------------------------------

namespace sl
{
namespace pi
{

// -------------------------------------------------------------------------------------------------

UserInterfaceCHIPPro::UserInterfaceCHIPPro(std::shared_ptr<Pink> pink_) : UserInterface(pink_)
{
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd < 0)
  {
    throw std::runtime_error("Could not open /dev/mem");
  }

  m_memoryMap
    = (uint8_t*)mmap(NULL, getpagesize() * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x01c20000);

  if (m_memoryMap == nullptr)
  {
    throw std::runtime_error("Could not initialize memory map");
  }

  close(fd);

  // Set memmap to point to PIO-registers
  m_memoryMap += 0x800;

  ::initPortE(m_memoryMap);
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
  setOutputPin(0, playing_);
}

// -------------------------------------------------------------------------------------------------

void UserInterfaceCHIPPro::clockMultiplierChanged(const std::string& cm_)
{
}

// -------------------------------------------------------------------------------------------------

void setOutputPin(uint8_t pin_, bool status_)
{
  if (pin_ < 8)
  {
    ::setPinPortE(m_memoryMap, pin_, status_);
  }
}

// -------------------------------------------------------------------------------------------------

} // namespace pi
} // namespace sl
