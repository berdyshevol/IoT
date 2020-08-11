echo start
cd ~/esp/esp-idf
. ./export.sh
alias BUILD="idf.py -p /dev/ttyUSB0  build"
alias FLASH="idf.py -p /dev/ttyUSB0  flash"
alias MONITOR="idf.py -p /dev/ttyUSB0  monitor"
cd ..
echo end
