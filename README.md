## QuickStart

1. Clone this repository
```bash
git clone https://github.com/nsry123/MedMinder
cd ./MedMinder
```

2. Configure ESPIDF Environment and Project
```bash
get_idf
idf.py menuconfig
```
Please ensure that the flash size is set to 4MB

3. Download Font to ESP32
```bash
python esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 write_flash -z 0x15D000 myFont.bin
```

4. Build and Flash
 ```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash
```
After flashing, restart the ESP32 and you are good to go!
