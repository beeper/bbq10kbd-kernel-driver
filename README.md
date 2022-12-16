# bbq10 Linux i2c keyboard driver

A Linux i2c keyboard driver for the bbq10 keyboard.  This can be used with a raspberry pi or radxa zero.

This kernel driver interfaces with the i2c protocol in this repo: https://github.com/solderparty/bbq10kbd_i2c_sw


## Radxa Zero Setup

### Keyboard Driver

##### Build Driver
```
cd ~
git clone https://github.com/beeper/bbq10kbd-kernel-driver.git
cd bbq10kbd-kernel-driver/
make modules modules_install keymap keymap_config
depmod -A
echo bbq10kbd >> /etc/modules
```

##### Setup Devicetree Overlay
```
dtc -@ -I dts -O dtb -o bbq10kbd-radxa-zero.dtbo dts/bbq10kbd-radxa-zero.dts 
cp bbq10kbd-radxa-zero.dtbo /boot/dtbs/5.10.69-12-amlogic-g98700611d064/amlogic/overlay/
```

Add these devicetree overlay's to `/boot/uEnv.txt`
```
overlays=meson-g12a-i2c-ee-m3-gpioa-14-gpioa-15 bbq10kbd-radxa-zero
```

## Raspberry Pi Zero Setup

### Keyboard Driver

##### Verify that you have the linux kernel headers for your platform
```
sudo apt-get install raspberrypi-kernel-headers
```

##### Clone the repo:
```
cd ~
git clone https://github.com/beeper/bbq10kbd-kernel-driver.git
```

##### Change the interrupt GPIO to 4 in [bbq10kbd.c](https://github.com/beeper/bbq10kbd-kernel-driver/blob/master/bbq10kbd.c)
```
#define INTERRUPT_GPIO 4
```

##### Build Driver
```
cd bbq10kbd-kernel-driver/
make modules modules_install keymap keymap_config
depmod -A
echo bbq10kbd >> /etc/modules
```

##### Setup Devicetree Overlay
```
dtc -@ -I dts -O dtb -o bbq10kbd-rpi.dtbo dts/bbq10kbd-rpi.dts 
cp bbq10kbd-rpi.dtbo /boot/overlays
```

##### Add the following line to /boot/config.txt:
```
dtoverlay=bbq10kbd-rpi
```

##### Reboot
```
shutdown -r now
```
