echo start
OLEG_PWD=$(pwd)
cd ~/esp/esp-idf
. ./export.sh
alias BUILD="idf.py -p /dev/ttyUSB0  build"
alias FLASH="idf.py -p /dev/ttyUSB0  flash"
alias MONITOR="idf.py -p /dev/ttyUSB0  monitor"
# cd ..
cd $OLEG_PWD
echo end
