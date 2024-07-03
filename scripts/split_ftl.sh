scripts/rpc.py bdev_split_create FTL0 8

scripts/rpc.py vhost_create_blk_controller --cpumask 0x000000000000000000800000000000000000 vhost.1 FTL0p0
scripts/rpc.py vhost_create_blk_controller --cpumask 0x000000000000000000800000000000000000 vhost.2 FTL0p1
scripts/rpc.py vhost_create_blk_controller --cpumask 0x000000000000000000800000000000000000 vhost.3 FTL0p2
scripts/rpc.py vhost_create_blk_controller --cpumask 0x000000000000000000800000000000000000 vhost.4 FTL0p3
scripts/rpc.py vhost_create_blk_controller --cpumask 0x000000000000000000000000000800000000 vhost.5 FTL0p4
scripts/rpc.py vhost_create_blk_controller --cpumask 0x000000000000000000800000000800000000 vhost.6 FTL0p5
scripts/rpc.py vhost_create_blk_controller --cpumask 0x000000000000000000800000000800000000 vhost.7 FTL0p6
scripts/rpc.py vhost_create_blk_controller --cpumask 0x000000000000000000800000000800000000 vhost.8 FTL0p7
