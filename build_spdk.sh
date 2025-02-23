#!/bin/bash

set -e

git submodule update --init
sudo apt-get update
sudo apt-get install nvme-cli
sudo nvme format --reset -b 4096 /dev/nvme1n1
nvme format /dev/nvme1 --namespace-id=1 --lbaf=4 --reset

./scripts/pkgdep.sh

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
sudo HUGEMEM=131072 PCI_ALLOWED=$PCI_ALLOWED ./scripts/setup.sh 

wget https://github.com/axboe/fio/archive/refs/tags/fio-3.39.tar.gz
tar -xvf fio-3.39.tar.gz
mv fio-fio-3.39 fio
pushd fio
./configure
make -j
popd

./configure --enable-debug --with-fio=./fio

# grand gbd sudo permission
mv /usr/bin/gdb /usr/bin/gdb-ori
cp scripts/gdb /usr/bin/gdb
chmod +x /usr/bin/gdb

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
