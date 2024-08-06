import configparser
import itertools
import os

FIO_JOB_FILE_BASE = "{}_{}_{}_op{}_{}.job"
SPDK_CONFIG_JSON_BASE = "/data/guntherxing/dev/spdk/exp/config/{}_{}_op{}.json"

BS = ["4k", "32k"]
WM = ["rand"]
OP = ["20", "40", "60"]
ALGO = ["single_group", "sepbit", "sepgc", "mida"]
CC = ["cb", "greedy"]
DIST = ["uni", "zipf:0.8", "zipf:1.2"]

def erase_space(filep):
    with open(filep, 'r') as file:
        lines = file.readlines()

    processed_lines = [line.replace(" = ", "=") for line in lines]

    with open(filep, 'w') as file:
        file.writelines(processed_lines)

for dist, algo, cc, bs, wm, op in itertools.product(DIST, ALGO, CC, BS, WM, OP):
    config = configparser.ConfigParser()
    config.read('template.job')

    job_file = FIO_JOB_FILE_BASE.format(cc, bs, wm, op, dist)
    spdk_json_conf = SPDK_CONFIG_JSON_BASE.format(algo, cc, op)
    config.set('global', 'spdk_json_conf', spdk_json_conf)
    if wm == "rand":
        config.set('job1', 'rw', 'randwrite')
    elif wm == "seq":
        config.set('job1', 'rw', "write")
    else:
        raise RuntimeError
    
    if dist != "uni":
        config.set('global', "random_distribution", dist)
    
    config.set('global', 'bs', bs)

    if not os.path.exists(algo):
        os.makedirs(algo)
    
    job_file_p = os.path.join(algo, job_file)
    
    with open(job_file_p, "w") as configf:
        config.write(configf)
    erase_space(job_file_p)
    