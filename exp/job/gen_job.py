import configparser
import itertools
import os

FIO_JOB_FILE_BASE = "{}_{}_op{}_{}.job"
SPDK_CONFIG_JSON_BASE = "/data/guntherxing/dev/spdk/exp/config/{}_ftl_op{}.json"

BS = ["4k", "16k", "32k", "64k", "128k"]
WM = ["rand", "seq"]
OP = ["20", "40", "60"]
ALGO = ["random_group", "single_group", "sepbit", "sepgc", "mida"]
DIST = ["uni", "zipf:0.8", "zipf:1.2"]

def erase_space(filep):
    with open(filep, 'r') as file:
        lines = file.readlines()

    processed_lines = [line.replace(" = ", "=") for line in lines]

    with open(filep, 'w') as file:
        file.writelines(processed_lines)

for dist, algo, bs, wm, op in itertools.product(DIST, ALGO, BS, WM, OP):
    config = configparser.ConfigParser()
    config.read('template.job')

    job_file = FIO_JOB_FILE_BASE.format(bs, wm, op, dist)
    spdk_json_conf = SPDK_CONFIG_JSON_BASE.format(algo, op)
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
    