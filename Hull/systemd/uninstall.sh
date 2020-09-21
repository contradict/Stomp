#!/bin/bash

THISDIR=$(dirname "$(readlink -f "$0")")
sudo systemctl stop stomp.target
sudo systemctl stop leg_control.service
sudo systemctl stop sbus_radio.service
sudo systemctl disable "${THISDIR}/stomp.target"
sudo systemctl disable "${THISDIR}/leg_control.service"
sudo systemctl disable "${THISDIR}/sbus_radio.service"
