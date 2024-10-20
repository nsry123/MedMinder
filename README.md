[中文](https://github.com/nsry123/MedMinder/blob/main/README_CN.md) [English](https://github.com/nsry123/MedMinder/blob/main/README.md) 
## MedMinder
During many interviews with Chinese senior citizens, many reported that they find a physical device specifically designed for medical notifications more reliable and desirable. Based on this feedback, I am developing a physical intake notifier using ESP32 that can synchronize with the application by simply scanning a QR code. The MedMinder work in cooperatioon with the [MedBox application](https://github.com/nsry123/MedBox), and can be synchronised with ease. Alarms and notifications will be significantly more reliably delivered than phone application, providing users with a more concrete experience.
The following are screenshots of the UI layout:

<div class="main">
    <img src="https://github.com/nsry123/MedMinder/blob/main/images/%E4%B8%BB%E9%A1%B5.webp" width="300"/>&nbsp&nbsp<img src="https://github.com/nsry123/MedMinder/blob/main/images/%E8%8D%AF%E5%93%81%E5%88%97%E8%A1%A8.webp" width="300"/>&nbsp&nbsp<img src="https://github.com/nsry123/MedMinder/blob/main/images/%E8%8D%AF%E5%93%81%E8%AF%A6%E6%83%85.webp" width="300"/>
</div>

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
