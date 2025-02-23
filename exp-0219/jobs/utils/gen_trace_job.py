import configparser
import itertools
import os
import sys

HOME_DIR = os.path.expanduser("~")
JOB_TEMPLATE_FILE = f"{HOME_DIR}/spdk/exp-0219/jobs/utils/template_trace.job"
JOB_OUT_DIR = f"{HOME_DIR}/spdk/exp-0219/jobs"
FIO_JOB_FILE_BASE = "TRACE_ALGO_{}_OP_{}.job"
SPDK_CONFIF_DIR = f"{HOME_DIR}/spdk/exp-0219/configs"
SPDK_CONFIG_JSON_BASE = "ftl_algo_{}_OP_{}.json"

OP = ["14"]

ALGO = ["sepbit22_cb",
        "sepbit24_cb",
        "sepbit44_cb",
        "sepbit46_cb",
        "mida22_cb",
        "mida24_cb",
        "mida44_cb",
        "mida46_cb"]

def erase_space(filep):
    with open(filep, 'r') as file:
        lines = file.readlines()

    processed_lines = [line.replace(" = ", "=") for line in lines]

    with open(filep, 'w') as file:
        file.writelines(processed_lines)

for algo, op in itertools.product(ALGO, OP):
    config = configparser.ConfigParser()
    config.read(JOB_TEMPLATE_FILE)

    job_file = FIO_JOB_FILE_BASE.format(algo, op)
    spdk_json_conf = SPDK_CONFIG_JSON_BASE.format(algo, op)
    spdk_json_conf = os.path.join(SPDK_CONFIF_DIR, spdk_json_conf)
    config.set('global', 'spdk_json_conf', spdk_json_conf)
    
    job_file_p = os.path.join(JOB_OUT_DIR, job_file)
    
    with open(job_file_p, "w") as configf:
        config.write(configf)
    erase_space(job_file_p)
    