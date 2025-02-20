import configparser
import itertools
import os

JOB_TEMPLATE_FILE = "/home/xgj/spdk/exp-0219/jobs/utils/template.job"
JOB_OUT_DIR = "/home/xgj/spdk/exp-0219/jobs"
FIO_JOB_FILE_BASE = "ALGO_{}_BS_{}_WP_{}_OP_{}_DIS_{}.job"
SPDK_CONFIG_JSON_BASE = "/home/xgj/spdk/exp-0219/configs/ftl_algo_{}_OP_{}.json"

BS = ["32k"]
WM = ["rand"]
OP = ["15", "30"]

ALGO = ["sepbit22_cb",
        "sepbit24_cb",
        "sepbit44_cb",
        "sepbit46_cb",
        "mida22_cb",
        "mida24_cb",
        "mida44_cb",
        "mida46_cb"]

DIST = ["zipf:0.8", "zipf:1.1"]

def erase_space(filep):
    with open(filep, 'r') as file:
        lines = file.readlines()

    processed_lines = [line.replace(" = ", "=") for line in lines]

    with open(filep, 'w') as file:
        file.writelines(processed_lines)

for dist, algo,  bs, wm, op in itertools.product(DIST, ALGO, BS, WM, OP):
    config = configparser.ConfigParser()
    config.read(JOB_TEMPLATE_FILE)

    job_file = FIO_JOB_FILE_BASE.format(algo, bs, wm, op, dist)
    spdk_json_conf = SPDK_CONFIG_JSON_BASE.format(algo, op)
    config.set('global', 'spdk_json_conf', spdk_json_conf)
    if wm == "rand":
        config.set('job1', 'rw', 'randwrite')
    elif wm == "seq":
        config.set('job1', 'rw', "write")
    else:
        raise RuntimeError
    
    if dist != "uni":
        config.set('job1', "random_distribution", dist)
    
    config.set('job1', 'bs', bs)
    
    job_file_p = os.path.join(JOB_OUT_DIR, job_file)
    
    with open(job_file_p, "w") as configf:
        config.write(configf)
    erase_space(job_file_p)
    