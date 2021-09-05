#!/bin/bash

THISDIR=$(dirname "$(readlink -f "$0")")
sudo systemctl link "${THISDIR}/sbus_radio.service"
sudo systemctl enable sbus_radio.service
sudo systemctl link "${THISDIR}/leg_control.service"
sudo systemctl enable leg_control.service
sudo systemctl link "${THISDIR}/stomp.target"
sudo systemctl enable stomp.target
sudo systemctl start stomp.target
