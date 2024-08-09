import configparser
import itertools
import os

FIO_JOB_FILE_BASE = "trace_{}_op{}.job"
SPDK_CONFIG_JSON_BASE = "/data/guntherxing/dev/spdk/exp/config/trace_{}_{}_op{}.json"

OP = ["20", "40", "60"]
ALGO = ["single_group", "sepbit", "sepgc", "mida"]
CC = ["cb", "greedy"]

def erase_space(filep):
    with open(filep, 'r') as file:
        lines = file.readlines()

    processed_lines = [line.replace(" = ", "=") for line in lines]

    with open(filep, 'w') as file:
        file.writelines(processed_lines)

for algo, cc, op in itertools.product(ALGO, CC, OP):
    config = configparser.ConfigParser()
    config.read('template_trace.job')

    job_file = FIO_JOB_FILE_BASE.format(cc, op)
    spdk_json_conf = SPDK_CONFIG_JSON_BASE.format(algo, cc, op)
    config.set('global', 'spdk_json_conf', spdk_json_conf)

    if not os.path.exists(algo):
        os.makedirs(algo)
    
    job_file_p = os.path.join(algo, job_file)
    
    with open(job_file_p, "w") as configf:
        config.write(configf)
    erase_space(job_file_p)
    