#!/bin/bash
LIMITS_CONF="/etc/security/limits.conf"

set -e

git submodule update --init
sudo apt-get update
sudo apt-get install nvme-cli
sudo nvme format --reset -b 4096 /dev/nvme1n1
nvme format /dev/nvme1 --namespace-id=1 --lbaf=4 --force --reset

./scripts/pkgdep.sh

if ! grep -q "${USER}.*memlock" "$LIMITS_CONF"; then
  echo -e "\n${USER}    soft    memlock    unlimited" >> "$LIMITS_CONF"
  echo -e "${USER}    hard    memlock    unlimited" >> "$LIMITS_CONF"

sudo PCI_ALLOWED=0000:c6:00.0 ./scripts/setup.sh 

./configure --enable-debug --with-fio=/dataset/fio
