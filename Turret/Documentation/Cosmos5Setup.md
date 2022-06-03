# Cosmos 5 Setup on Linux 

## Cosmos 5 requires a docker install

So, first step is to install Docker Engine

* Follow detailed instructions here: [Install Docker Engine on Ubuntu](https://docs.docker.com/engine/install/ubuntu/)
* which basically boils down to

```shell
$ sudo apt-get update
$ sudo apt-get install ca-certificates curl gnupg lsb-release
$ curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
$ echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
$ sudo apt-get update
$ sudo apt-get install docker-ce docker-ce-cli containerd.io

```

* To make sure docker is installed correctly:

```shell 
$ docker --version
```

* When originally written docker engine version is: 
  * `Docker version 20.10.12, build e91ed57`

* To ensure that docker doesn't need `sudo` before every commad do the following:

```shell
$ sudo groupadd docker
$ sudo usermod -aG docker $USER
```

* restart the machine

Also need to get docker-compose

* follow detaild instructions here: [Install Docker Compose](https://docs.docker.com/compose/install/)
* which basically boils down to

```shell
$ sudo curl -L "https://github.com/docker/compose/releases/download/1.29.2/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
$ sudo chmod +x /usr/local/bin/docker-compose
```
* To make sure docker-compose is installed correctly:

```shell
$ docker-compose --version
```

* When originally written docker engine version is:
  * `docker-compose version 1.29.2, build 5becea4c`

## Get Cosmos And Build Container

* Get the gzip file here: [COMOS 5](https://github.com/BallAerospace/COSMOS/releases)
* Extract in ~/
* Now just run the Cosmos scripts that do all the docker work for you

``` shell
$ cd ~/COSMOS-5.0.1
$ ./cosmos-control.sh start
```
NOTE: There were a ton of warnings and errors during initial "start".  Need to research and diagnose

## Connect to new Cosmos Instance

* Direct browser to: ```localhost:2900```

## Setup New Plugin and Targets

* First, remove the demo interfaces
  * From the browser running Cosmos 
    * Click the Admin button choose the PLUGINS tab
    * Click the Trash can icon next to cosmos-demo plugin
    * TODO: Need to find a way to remove that persists past a restart
* Make a directory for plugin and get cosmos to auto generate the skeleton content.
``` shell
$ mkdir plugins
$ ~/COSMOS-5.0.1/cosmos-control.sh cosmos generate plugin chomp-turret
```
* Edit ```plugin.txt```, ```targets/CHOMP_TURRET/cmd_tlm/cmd.txt``` and ```targets/CHOMP_TURRET/cmd_tlm/tlm.txt``` to configure the interface, command packets and telemetry packets
* Build the plugin
```shell
$ ~/COSMOS-5.0.1/cosmos-control.sh cosmos rake build VERSION=1.0.0
```
* Add new plugin to Cosmos
  * From the browser running Cosmos 
      * Click the Admin button choose the PLUGINS tab
      * Use paper-clip to browse to new plugin
      * Click the Upload button
  
## Tricky Setup So Docker Can Get To /dev/tty*

* First, need to creat ```/etc/udev/rules.d/99-usb-serial.rules``` and add a few things.
  * First thing to do a symlink to ttyFTDI* from the ttyUSB* that gets created when RFD 900x is connected to USB port (see ```/etc/udev/rules.d/99-usb-serial.rules```)
  * Second is set ```rw``` access on the /dev/ttyFTDI device so that docker can mount it (also in the udev rules file)
  * Finally, needed to modify Cosmos ```compose.yml``` file so that docker would mount /dev from host machine to the docker container AND run in privlaged mode

    volumes:
      - /dev:/dev
    privileged: true

## Perminatly Remove Demo Plugin

``` script
$ rm -rf cosmos-init/plugins/packages/cosmosc2-demo/
```

Now, need to edit Dockerfile to not build demo
