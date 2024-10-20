[中文](https://github.com/nsry123/MedMinder/blob/main/README_CN.md) [English](https://github.com/nsry123/MedMinder/blob/main/README.md) 
## MedMinder
在对身边老人的采访当中, 相较于MedBox App, 许多人更喜欢一款独立于App的用药提醒设备。基于这一反馈，我使用ESP32开发了一个物理用药提醒器，只需扫描二维码即可与[MedBox](https://github.com/nsry123/MedBox)应用程序同步。警报和通知将比手机应用通知更可靠地传递，为老年人用户提供更稳定和放心的体验。

## UI设计
<div class="main">
    <img src="https://github.com/nsry123/MedMinder/blob/main/images/%E4%B8%BB%E9%A1%B5.webp" width="300"/>&nbsp&nbsp<img src="https://github.com/nsry123/MedMinder/blob/main/images/%E8%8D%AF%E5%93%81%E5%88%97%E8%A1%A8.webp" width="300"/>&nbsp&nbsp<img src="https://github.com/nsry123/MedMinder/blob/main/images/%E8%8D%AF%E5%93%81%E8%AF%A6%E6%83%85.webp" width="300"/>
</div>




## 快速开始

1. 复制这个项目
```bash
git clone https://github.com/nsry123/MedMinder
cd ./MedMinder
```


2. 配置ESP-IDF
```bash
get_idf
idf.py menuconfig
```
请确保闪存大小设为4MB

3. 烧录字体
```bash
python esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 write_flash -z 0x15D000 myFont.bin
```

4. 编译并烧录
 ```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash
```
烧录后重启即可使用
