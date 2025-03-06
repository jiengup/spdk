#!/bin/bash

export NVME_DEV=/dev/nvme0
export PCI_ALLOWED=10000:81:00.0

./build_spdk.sh

# python3 exp-0219/configs/utils/gen_config.py -t template_c6525.json
# python3 exp-0219/jobs/utils/gen_job.py