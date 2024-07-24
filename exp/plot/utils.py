import subprocess
import re

def cat_and_grep(filename: str, pattern: str):
    command = f"cat {filename} | grep '{pattern}'"
    result = result = subprocess.run(command, capture_output=True, text=True, shell=True)
    if result.returncode != 0:
        print("Error!")
        if result.stderr:
            print(result.stderr)
    return result.stdout

def extract_stat_io(filename: str, item: str):
    grep_pattern = f"\[STAT_{item}\]"
    extracted_stat = cat_and_grep(filename, grep_pattern)
    extracted_stat = extracted_stat.split("\n")
    extracted_stat = [line for line in extracted_stat if line != ""]
    read_IOPS_pattern = r"read: IOPS (\d+)"
    write_IOPS_pattern = r"write: IOPS (\d+)"
    read_bw_pattern = r"read: IOPS \d+, blocks (\d+)"
    write_bw_pattern = r"write: IOPS \d+, blocks (\d+)"
    r_iops = []
    w_iops = []
    r_bw = []
    w_bw = []
    for line in extracted_stat:
        if re.search(read_IOPS_pattern, line):
            r_iops.append(int(re.search(read_IOPS_pattern, line).group(1)))
        else:
            print(f"cant match read IOPS: {line}")
        if re.search(write_IOPS_pattern, line):
            w_iops.append(int(re.search(write_IOPS_pattern, line).group(1)))
        else:
            print(f"cant search write IOPS: {line}")
        if re.search(read_bw_pattern, line):
            r_bw.append(int(re.search(read_bw_pattern, line).group(1)))
        else:
            print(f"cant search read BW: {line}")
        if re.search(write_bw_pattern, line):
            w_bw.append(int(re.search(write_bw_pattern, line).group(1)))
        else:
            print(f"cant search write BW: {line}")
    return {"read_IOPS": r_iops, "write_IOPS": w_iops, "read_bw": r_bw, "write_bw": w_bw}

def extract_stat_waf(filename: str):
    grep_pattern = f"\[STAT_WAF\]"
    extracted_stat = cat_and_grep(filename, grep_pattern)
    extracted_stat = extracted_stat.split("\n")
    extracted_stat = [line for line in extracted_stat if line != ""]
    valid_waf_pattern = r"\[STAT_WAF\] ([-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)"
    invalid_waf_pattern = r"\[STAT_WAF\] ([-+]?nan)"
    
    wafs = []
    for line in extracted_stat:
        if re.search(valid_waf_pattern, line):
            waf = float(re.search(valid_waf_pattern, line).group(1))
            wafs.append(waf)
        elif re.search(invalid_waf_pattern, line):
            wafs.append(0)
        else:
            print(f"cant search waf: {line}")
    return wafs

def extract_overall_waf(filename: str):
    grep_pattern = f"\[FTL\]\[ftl0\] WAF:"
    extracted_stat = cat_and_grep(filename, grep_pattern)
    extracted_stat = extracted_stat.split("\n")
    extracted_stat = [line for line in extracted_stat if line != ""]
    valid_waf_pattern = r"WAF:\s+([-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)"
    invalid_waf_pattern = r"WAF:\s+ ([-+]?nan)"
    
    wafs = []
    for line in extracted_stat:
        if re.search(valid_waf_pattern, line):
            waf = float(re.search(valid_waf_pattern, line).group(1))
            wafs.append(waf)
        elif re.search(invalid_waf_pattern, line):
            wafs.append(0)
        else:
            print(f"cant search waf: {line}")
    return wafs, wafs[-1]

def extract_fio(result_f):
    with open(result_f, "r") as f:
        lines = f.readlines()
        for idx, line in enumerate(lines):
            line = line.strip()
            if line.startswith("job1: (groupid="):
                stat_line = lines[idx+1]
                lat_line = lines[idx+4]
                break
    iops_pattern = r"IOPS=(\d+)k"
    if re.search(iops_pattern, stat_line):
        iops = int(re.search(iops_pattern, stat_line).group(1))
    else:
        print(f"cant search iops: {stat_line}")
    bw_pattern = r"BW=\d+MiB/s \((\d+)MB/s\)"
    if re.search(bw_pattern, stat_line):
        bw = int(re.search(bw_pattern, stat_line).group(1))
    else:
        print(f"cant search bw: {stat_line}")
    latency_pattern = r"avg=(\d+)"
    if re.search(latency_pattern, lat_line):
        latency = int(re.search(latency_pattern, lat_line).group(1))
    else:
        print(f"cant search latency: {lat_line}")
    return iops, bw, latency