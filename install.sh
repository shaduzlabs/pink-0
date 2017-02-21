        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #######

if [ ! -d "/home/pi/" ];
  then
    echo ""
    echo -e "[ \033[1m\033[31mERROR\033[m ] This install script can only be run on a Raspberry Pi!"
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
echo -e "[ \033[1m\033[96mpink\033[m ] Update submodules -----------------------------------------------------"
git submodule update --init --recursive

echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Install dependencies --------------------------------------------------"
sudo apt-get install -y --force-yes cmake wiringpi

echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Install RaspAP --------------------------------------------------------"
wget -q https://git.io/vDr0i -O /tmp/raspap && bash /tmp/raspap

echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Build and install portaudio -------------------------------------------"
cd modules/portaudio
./configure --with-alsa --without-oss --without-jack
make
sudo make install
cd ../../

echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Build pink and install the binary and the daemon configuration file ---"
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
cd ..

echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Update device configuration -------------------------------------------"
sudo cp --backup=numbered support/hostapd.conf /etc/hostapd/.
sudo cp --backup=numbered support/boot/config.txt /boot/.

echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Optimize system configuration -----------------------------------------"
echo -n performance | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

echo ""
echo -e "[ \033[1m\033[96mpink\033[m ] Installation completed, please reboot now."
