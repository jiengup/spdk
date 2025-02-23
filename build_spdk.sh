#!/bin/bash

set -e

git submodule update --init
sudo apt-get update
sudo apt-get install -y nvme-cli
sudo apt-get install -y clangd

sudo ./scripts/pkgdep.sh

sudo ./scripts/setup.sh reset

: ${NVME_DEV:=""}
if [ -n "$NVME_DEV" ]; then
  while [ ! -e "${NVME_DEV}n1" ]; do
    echo "${NVME_DEV}n1 not found. Waiting..."
    sleep 1
  done
  echo "Initializing NVMe disk..."
  sudo nvme format --force --reset -b 4096 "${NVME_DEV}n1"
fi

: ${PCI_ALLOWED:=""}
if [ -n "PCI_ALLOWED" ]; then
  sudo HUGEMEM=131072 PCI_ALLOWED=$PCI_ALLOWED ./scripts/setup.sh || true
else
  echo "Warning: dont setup any PCIE NVMe SSD"
fi

wget https://github.com/axboe/fio/archive/refs/tags/fio-3.39.tar.gz
tar -xvf fio-3.39.tar.gz
mv fio-fio-3.39 fio
pushd fio
./configure
make -j
popd
rm -f fio-3.39.tar.gz

./configure --enable-debug --with-fio=./fio

chunk_mb=256
chunk_blocks=$((chunk_mb * 1024 * 1024 / 4096))
SPDK_FTL_ZONE_EMU_BLOCKS=${chunk_blocks} make -j

# install vscode extensions
if command -v code &> /dev/null; then
  code --install-extension ms-vscode.cpptools 
  code --install-extension llvm-vs-code-extensions.vscode-clangd 
  code --install-extension GitHub.copilot
  code --install-extension GitHub.copilot-chat
fi
