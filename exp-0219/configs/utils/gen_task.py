import json
import os
import argparse
import configparser
from datetime import datetime
import pytz

shanghai_tz = pytz.timezone('Asia/Shanghai')
shanghai_time = datetime.now(shanghai_tz)
TIME = shanghai_time.strftime("%Y-%m-%d-%H:%M")

parser = argparse.ArgumentParser(description="Exp runner generate script")
parser.add_argument(
    "-t", "--template", type=str, help="spdk config template to use", required=True
)
parser.add_argument(
    "-a", "--algo", type=str, nargs="+", help="ftl algorithm to use", required=True
)
parser.add_argument(
    "-o", "--op", type=int, nargs="+", help="ftl overprovisioning to use", required=True
)
parser.add_argument(
    "-i", "--read_iolog", type=str, nargs="+", help="trace file for fio replay"
)

parser.add_argument(
    "-n", "--bdev_num", type=int, help="number of bdevs to run"
)

args = parser.parse_args()

EXP_DIR = f"{args.template.split('.json')[0]}_{TIME}"

HOME_DIR = os.path.expanduser("~")
TEMPLATE_DIR = f"{HOME_DIR}/spdk/exp-0219/configs/{EXP_DIR}"
TEMPLATE_FILE = f"{HOME_DIR}/spdk/exp-0219/configs/utils/{args.template}"

FIO_SPDK_BDEV_ENGINE = f"{HOME_DIR}/spdk/build/fio/spdk_bdev"
FIO_JOB_TEMPLATE_FILE = f"{HOME_DIR}/spdk/exp-0219/jobs/utils/template.job"
FIO_TRACE_JOB_TEMPLATE_FILE = f"{HOME_DIR}/spdk/exp-0219/jobs/utils/template_trace.job"
FIO_MULTI_TRACE_JOB_TEMPLATE_FILE = f"{HOME_DIR}/spdk/exp-0219/jobs/utils/template_multi_trace.job"
FIO_JOB_OUT_DIR = f"{HOME_DIR}/spdk/exp-0219/jobs/{EXP_DIR}"

SCRIPT_DIR = f"{HOME_DIR}/spdk/exp-0219/scripts"
RESULT_DIR = f"{HOME_DIR}/spdk/exp-0219/results/{EXP_DIR}"

if not os.path.exists(TEMPLATE_DIR):
    os.makedirs(TEMPLATE_DIR, exist_ok=True)

if not os.path.exists(FIO_JOB_OUT_DIR):
    os.makedirs(FIO_JOB_OUT_DIR, exist_ok=True)

if not os.path.exists(RESULT_DIR):
    os.makedirs(RESULT_DIR, exist_ok=True)

ALLOWED_ALGO = {
    "single_group_cb": 1,
    "sepgc11_cb": 2,
    "sepgc21_cb": 3,
    "sepbit22_cb": 4,
    "sepbit23_cb": 5,
    "sepbit24_cb": 6,
    "sepbit26_cb": 8,
    "sepbit44_cb": 8,
    "sepbit46_cb": 10,
    "mida22_cb": 4,
    "mida23_cb": 5,
    "mida24_cb": 6,
    "mida26_cb": 8,
    "mida44_cb": 8,
    "mida46_cb": 10,
}
ALLOWED_OP = [7, 14, 21]

# check if algo and op are valid
for algo in args.algo:
    if algo not in ALLOWED_ALGO:
        raise ValueError(f"Invalid algo: {algo}")
for op in args.op:
    if op not in ALLOWED_OP:
        raise ValueError(f"Invalid op: {op}")

def load_template():
    with open(TEMPLATE_FILE, "r") as f:
        return json.load(f)

def set_ftl_op(data, ftl_op):
    change = False
    for subsystem in data["subsystems"]:
        if subsystem["subsystem"] == "bdev":
            for config in subsystem["config"]:
                if config["method"] == "bdev_ftl_create":
                    config["params"]["overprovisioning"] = ftl_op
                    change = True
    if not change:
        raise ValueError("No FTL subsystem found")

def set_ftl_algo(data, ftl_algo):
    change = False
    for subsystem in data["subsystems"]:
        if subsystem["subsystem"] == "bdev":
            for config in subsystem["config"]:
                if config["method"] == "bdev_ftl_create":
                    config["params"]["algo"] = ftl_algo
                    change = True
    if not change:
        raise ValueError("No FTL subsystem found")

def set_ftl_group_num(data, ftl_group_num):
    change = False
    for subsystem in data["subsystems"]:
        if subsystem["subsystem"] == "bdev":
            for config in subsystem["config"]:
                if config["method"] == "bdev_ftl_create":
                    config["params"]["group_num"] = ftl_group_num
                    change = True

    if not change:
        raise ValueError("No FTL subsystem found")

spdk_config_files = []

# generate spdk config files
for algo in args.algo:
    group_num = ALLOWED_ALGO[algo]
    for op in args.op:
        data = load_template()
        set_ftl_op(data, op)
        set_ftl_algo(data, algo)
        set_ftl_group_num(data, group_num)
        spdk_config_file = f"{TEMPLATE_DIR}/ALGO_{algo}_OP_{op}.json"
        spdk_config_files.append((spdk_config_file, f"ALGO_{algo}_OP_{op}"))
        with open(
            spdk_config_file,
            "w",
        ) as f:
            json.dump(data, f, indent=2)

def erase_space(filep):
    with open(filep, "r") as file:
        lines = file.readlines()

    processed_lines = [line.replace(" = ", "=") for line in lines]

    with open(filep, "w") as file:
        file.writelines(processed_lines)

# generate fio jobs
fio_job_files = []
result_files = []

for (spdk_config_file, param) in spdk_config_files:
    config = configparser.ConfigParser()
    if args.bdev_num:
        if (args.bdev_num != len(args.read_iolog)):
            raise ValueError("Number of bdevs and iologs do not match")
        
        job_template_file = FIO_MULTI_TRACE_JOB_TEMPLATE_FILE
        config.read(job_template_file)
        config.set("global", "spdk_json_conf", spdk_config_file)
        config.set("global", "ioengine", FIO_SPDK_BDEV_ENGINE)

        for i, iolog in enumerate(args.read_iolog):
            config.set(f"trace{i}", f"read_iolog", iolog)

        job_file = f"{FIO_JOB_OUT_DIR}/MULTI_TRACE_{param}.job"

    else:
        if args.read_iolog:
            assert(len(args.read_iolog) == 1)
            read_iolog = os.path.expanduser(args.read_iolog[0])
            job_template_file = FIO_TRACE_JOB_TEMPLATE_FILE
            config.read(job_template_file)

            config.set("global", "spdk_json_conf", spdk_config_file)
            config.set("global", "ioengine", FIO_SPDK_BDEV_ENGINE)
            config.set("cbs-trace", "read_iolog", read_iolog)

            job_file = f"{FIO_JOB_OUT_DIR}/TRACE_{param}.job"
        else:
            job_template_file = FIO_JOB_TEMPLATE_FILE
            config.read(job_template_file)

            config.set("global", "spdk_json_conf", spdk_config_file)
            config.set("global", "ioengine", FIO_SPDK_BDEV_ENGINE)

            job_file = f"{FIO_JOB_OUT_DIR}/{param}.job"

    with open(job_file, "w") as configf:
        config.write(configf)
    fio_job_files.append(job_file)
    result_file = f"{RESULT_DIR}/{param}.out"
    result_files.append(result_file)
    erase_space(job_file)

task_file = os.path.join(SCRIPT_DIR, f"{EXP_DIR}")
with open(task_file, "w") as f:
    for job_file, result_file in zip(fio_job_files, result_files):
        print(job_file, result_file)
        f.write(f"{job_file} {result_file}\n")
print(task_file)