import configparser
import itertools

FIO_JOB_FILE_BASE = "ftl_fio_{}_{}_{}.job"
SPDK_CONFIG_JSON_BASE = "/data/guntherxing/dev/spdk/exp/config/ftl_op{}.json"

BS = ["4k", "64k", "128k"]
WM = ["rand", "seq"]
OP = ["20", "40", "60"]

def erase_space(filep):
    with open(filep, 'r') as file:
        lines = file.readlines()

    processed_lines = [line.replace(" = ", "=") for line in lines]

    with open(filep, 'w') as file:
        file.writelines(processed_lines)

for bs, wm, op in itertools.product(BS, WM, OP):
    config = configparser.ConfigParser()
    config.read('template.job')

    job_file = FIO_JOB_FILE_BASE.format(bs, wm, op)
    spdk_json_conf = SPDK_CONFIG_JSON_BASE.format(op)
    config.set('global', 'spdk_json_conf', spdk_json_conf)
    if wm == "rand":
        config.set('job1', 'rw', 'randwrite')
    elif wm == "seq":
        config.set('job1', 'rw', "write")
    else:
        raise RuntimeError
    config.set('global', 'bs', bs)
    
    with open(job_file, "w") as configf:
        config.write(configf)
    erase_space(job_file)
    