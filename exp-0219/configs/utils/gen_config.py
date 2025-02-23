import json
import os
import argparse

parser = argparse.ArgumentParser(description='config generate script')
parser.add_argument('-t', '--template', type=str, help='spdk config template to use', required=True)
args = parser.parse_args()

HOME_DIR = os.path.expanduser("~")
TEMPLATE_FILE = f"{HOME_DIR}/spdk/exp-0219/configs/utils/{args.template}"

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
    
ALGO = ["sepbit22_cb", 
        "sepbit24_cb", 
        "sepbit44_cb", 
        "sepbit46_cb",
        "mida22_cb",
        "mida24_cb",
        "mida44_cb",
        "mida46_cb"]

GROUP_NUM = [4, 6, 8, 10, 4, 6, 8, 10]
OP = [14, 21]

for algo, group_num in zip(ALGO, GROUP_NUM):
    for op in OP:
        data = load_template()
        set_ftl_op(data, op)
        set_ftl_algo(data, algo)
        set_ftl_group_num(data, group_num)
        with open(f"{HOME_DIR}/spdk/exp-0219/configs/ftl_algo_{algo}_OP_{op}.json", "w") as f:
            json.dump(data, f, indent=2)
