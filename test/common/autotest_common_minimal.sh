
rpc_py=$rootdir/scripts/rpc.py

function waitforbdev() {
	local bdev_name=$1
	local bdev_timeout=$2
	local i
	[[ -z ${bdev_timeout:-} ]] && bdev_timeout=2000 # ms

	$rpc_py bdev_wait_for_examine

	if $rpc_py bdev_get_bdevs -b $bdev_name -t $bdev_timeout; then
		return 0
	fi

	return 1
}