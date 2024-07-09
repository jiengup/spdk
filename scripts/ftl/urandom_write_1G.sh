curdir=$(readlink -f $(dirname $0))
testdir=$(readlink -f $curdir/../../test)
rootdir=$(readlink -f $testdir/..)
SPDK_BIN_DIR=$rootdir/build/bin
source $rootdir/test/common/autotest_common_minimal.sh
source $testdir/common.sh

rpc_py=$rootdir/scripts/rpc.py
spdk_dd="$SPDK_BIN_DIR/spdk_dd"

modprobe nbd
$rpc_py nbd_start_disk ftl0 /dev/nbd0

block_size=4096
chunk_size=262144
data_size=$chunk_size

$spdk_dd -m 0x4 --if=/dev/urandom --of=$testdir/testfile --bs=$block_size --count=$data_size
md5sum $testdir/testfile > $testdir/testfile.md5
$spdk_dd -m 0x4 --if=$testdir/testfile --of=/dev/nbd0 --bs=$block_size --count=$data_size --oflag=direct
sync /dev/nbd0
