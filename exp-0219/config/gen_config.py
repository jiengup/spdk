import json
import os

TEMPLATE_FILE = "/home/xgj/spdk/exp-0219/configs/template.json"
OUT_DIR = "/home/xgj/spdk/exp-0219/configs"

def load_template():
    with open(TEMPLATE_FILE, "r") as f:
        return json.load(f)

def set_ftl_op(data, ftl_op):
    change = False
    for subsystem in data['subsystems']:
        if subsystem['subsystem'] == 'bdev':
            for config in subsystem['config']:
                if config['method'] == 'bdev_ftl_create':
                    config['params']['overprovisioning'] = ftl_op
                    change = True
    if not change:
        raise ValueError("No FTL subsystem found")
                    
def set_ftl_algo(data, ftl_algo):
    change = False
    for subsystem in data['subsystems']:
        if subsystem['subsystem'] == 'bdev':
            for config in subsystem['config']:
                if config['method'] == 'bdev_ftl_create':
                    config['params']['algo'] = ftl_algo
                    change = True
    if not change:
        raise ValueError("No FTL subsystem found")

def set_ftl_group_num(data, ftl_group_num):
    change = False
    for subsystem in data['subsystems']:
        if subsystem['subsystem'] == 'bdev':
            for config in subsystem['config']:
                if config['method'] == 'bdev_ftl_create':
                    config['params']['group_num'] = ftl_group_num
                    change = True
                    
    if not change:
        raise ValueError("No FTL subsystem found")
    
ALGO = ["single_group_greedy", "sepbit_gc"]
GROUP_NUM = [1, 6]
OP = [7, 10, 15, 20]

for algo, group_num in zip(ALGO, GROUP_NUM):
    for op in OP:
        data = load_template()
        set_ftl_op(data, op)
        set_ftl_algo(data, algo)
        set_ftl_group_num(data, group_num)
        with open(os.path.join(OUT_DIR, f"ftl_algo_{algo}_OP_{op}.json"), "w") as f:
            json.dump(data, f, indent=2)
