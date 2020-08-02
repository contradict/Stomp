# BeagleBone AI setup notes

Comes from TI with account `debian:tempwd`, hostname beaglebone.

## Set debian password

    ssh debian@beaglebone
    passwd

## Packages

    sudo apt update && sudo apt upgrade
    sudo apt install gdb stow cmake \
        liblcm-dev liblcm-bin libglib2.0-dev \
        libtool-bin

## Upgrade scripts

Must use `debian` account. Copied from [here](https://beagleboard.org/upgrade)

    cd /opt/scripts
    git pull
    sudo tools/update_kernel.sh

## Set hostname

The preferred hostname is `chomp-hull`

    sudo vim /etc/hostname

## User Account for developers

    sudo adduser <username>
    GRPS=$(groups | cut -d ' ' -f 2- | tr ' ' ',')
    sudo usermod -a -G $GRPS <username>

## Account for robot operation

    sudo adduser chomp
    GRPS=$(groups | cut -d ' ' -f 2- | tr ' ' ',')
    sudo usermod -a -G $GRPS <username>

## Reboot

If the kernel was upgraded above, a reboot is necessary before building the
devicetree overlays.

## Use new account

Unless you reboot, still has `beaglebone` hostname.

    exit
    ssh-copy-id <username>@beaglebone

Enter password, then ssh in with keys.

## Custom Devicetree

Builds custom device tree overlays for UART3, UART5, UART10, SPI3, ADC, and I2C
IMU. This should use your developer account.

    cd ~
    mkdir src
    cd src
    git clone git@github.com:contradict/BeagleBoard-DeviceTrees
    cd BeagleBoard-DeviceTrees
    git checkout -b chomp
    make

Creates the files

    src/arm/BBAI-SPI3-ADC-IMU.dtb
    src/arm/BBAI-UART10.dtb
    src/arm/BBAI-UART3.dtb
    src/arm/BBAI-UART5.dtb

Need to copy the files to `/lib/firmware/`

    sudo cp src/arm/BBAI-*.dtb /lib/firmware

## Config files

`uEnv.txt` needs to be updated, the lines at the end installing the DTB overlays
are the important part, be sure not to overwrite any kernel version parameters
earlier in the file.

    90-mikrobus-cape.rules -> /etc/udev/rules.d/

## Reboot

Again. To use the new dtb overlays.

    sudo reboot

## Fetch and build libmodbus

The version in the debian repositories is old and busted.

    cd src
    wget https://libmodbus.org/releases/libmodbus-3.1.6.tar.gz
    tar -xzf libmodbus-3.1.6.tar.gz
    cd libmodbus-3.1.6
    ./configure --prefix=/usr/local/
    make
    sudo make install DESTDIR=/usr/local/stow/libmodbus-3.1.6
    sudo mv /usr/local/stow/libmodbus-3.1.6/usr/local/* /usr/local/stow/libmodbus-3.1.6
    sudo rm -r /usr/local/stow/libmodbus-3.1.6/usr
    sudo stow -d /usr/local/stow libmodbus-3.1.6

## Fetch and build project

    cd src
    git clone git@github.com:contradict/Stomp
    cd Stomp/Platform
    make -C Test
    make -C SimpleGait
