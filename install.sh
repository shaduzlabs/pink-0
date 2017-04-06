        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #######

#!/bin/bash

platform='unknown'
unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
  if [ ! -d "/home/pi/" ];
    then
      platform='linux'
    else
      platform='linux-rpi'
  fi
elif [[ "$unamestr" == 'Darwin' ]];
  then
    which -s brew
    if [[ $? != 0 ]] ;
    then
      echo -e "[ \033[1m\033[31mERROR\033[m ] Homebrew is required but it is not installed"
      exit -2
    fi
    platform='osx'
fi

if [ $platform == "unknown" ];
  then
    echo ""
    echo -e "[ \033[1m\033[31mERROR\033[m ] This install script is not compatible with the current platform."
    echo ""
   exit -1
fi

echo ""
echo ""
echo -e "\033[33m                               ("
echo -e "                             ) )"
echo -e "                            ( (  ))                "
echo -e "                             ) )("
echo -e "                              ))       ))),,        "
echo -e "                              (       /  ///       "
echo -e "                                     _ _  /"
echo -e "                             _U___  <     )  ()"
echo -e "                            / /      \_- |. //"
echo -e "                            \_|       |,-\"|//"
echo -e "                             __     ,-\"   //"
echo -e "                            /  \ ,-\"     //"
echo -e "               _  .  --. ,--\,\" \"---    //"
echo -e "           /\_/ \| \"\"  -\"    \"-.       //"
echo -e "           \  \  |                    //|"
echo -e "          ,_\________________________//||"
echo -e "          '--------------------------, ||                    utis"
echo -e "             //                      \\\\||"
echo -e "            //                        \\\\|"
echo ""
echo -e "          \033[1m\033[96mSit down and relax, this will take some time...\033[m"
echo ""

echo ""
echo ""
echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Update submodules -----------------------------------------------------"
git submodule update --init --recursive

echo ""
echo ""
echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Install dependencies --------------------------------------------------"
if [ $platform == "osx" ];
  then
    brew install cmake
  else
    sudo apt-get update
    sudo apt-get install -y --force-yes libasound-dev cmake wiringpi
fi

echo ""
echo ""
echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Install RaspAP --------------------------------------------------------"
if [ $platform == "linux-rpi" ];
  then
    wget -q https://git.io/vDr0i -O /tmp/raspap && bash /tmp/raspap
fi

echo ""
echo ""
echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Build and install portaudio -------------------------------------------"
cd modules/portaudio
./configure --with-alsa --without-oss --without-jack
make
sudo make install
cd ../../
echo a

echo ""
echo ""
echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Build and install nanomsg ---------------------------------------------"
cd modules/nanomsg
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
cd ../../../


echo ""
echo ""
echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Build pink and install the binary and the daemon configuration file ---"
rm -rf build
mkdir build
cd build
if [ $1 == "no-ui" ]
  then
    cmake -DCMAKE_BUILD_TYPE=Release -DUSE_PI_ZERO=OFF -DUSE_WEBSOCKET=ON -DJUST_INSTALL_CEREAL=ON ..
  else
    echo -e "[ \033[1m\033[96mpink\033[m ] Raspberry Pi Zero shield support is enabled ---------------------------"
    cmake -DCMAKE_BUILD_TYPE=Release -DUSE_PI_ZERO=ON -DUSE_WEBSOCKET=ON -DJUST_INSTALL_CEREAL=ON ..
fi
make
if [ $platform == "linux-rpi" ];
  then
    sudo make install
    sudo update-rc.d pink defaults
fi
cd ..

echo ""
echo ""
echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Update device configuration -------------------------------------------"
if [ $platform == "linux-rpi" ];
  then
    sudo cp --backup=numbered support/hostapd.conf /etc/hostapd/.
    sudo cp -r support/html/* /var/www/html/.
fi

echo ""
echo ""
echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Optimize system configuration -----------------------------------------"
if [ $platform == "linux-rpi" ];
  then
    echo -n performance | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
fi

echo ""
echo ""
echo ""
if [ $platform == "linux-rpi" ];
  then
    echo -e "[ \033[1m\033[96mpink\033[m ] Installation completed, please reboot now."
  else
    echo -e "[ \033[1m\033[96mpink\033[m ] Build completed"
fi
echo ""
echo ""
echo ""
