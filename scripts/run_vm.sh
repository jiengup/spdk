qemu-system-x86_64 -m 16384 -smp 64 -cpu host -enable-kvm -hda centos.qcow2 -nographic -pidfile /var/run/qemu_0 -object memory-backend-file,id=mem,size=16G,mem-path=/dev/hugepages,share=on -numa node,memdev=mem \
 -chardev socket,id=char0,path=/var/tmp/vhost.1 -device vhost-user-blk-pci,num-queues=8,id=blk0,chardev=char0 \
 -chardev socket,id=char1,path=/var/tmp/vhost.2 -device vhost-user-blk-pci,num-queues=8,id=blk1,chardev=char1 \
 -chardev socket,id=char2,path=/var/tmp/vhost.3 -device vhost-user-blk-pci,num-queues=8,id=blk2,chardev=char2 \
 -chardev socket,id=char3,path=/var/tmp/vhost.4 -device vhost-user-blk-pci,num-queues=8,id=blk3,chardev=char3 \
 -chardev socket,id=char4,path=/var/tmp/vhost.5 -device vhost-user-blk-pci,num-queues=8,id=blk4,chardev=char4 \
 -chardev socket,id=char5,path=/var/tmp/vhost.6 -device vhost-user-blk-pci,num-queues=8,id=blk5,chardev=char5 \
 -chardev socket,id=char6,path=/var/tmp/vhost.7 -device vhost-user-blk-pci,num-queues=8,id=blk6,chardev=char6 \
 -chardev socket,id=char7,path=/var/tmp/vhost.8 -device vhost-user-blk-pci,num-queues=8,id=blk7,chardev=char7 \
 -net nic -net tap,ifname=tap0,script=no,downscript=no

