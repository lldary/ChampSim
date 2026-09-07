#!/usr/bin/env python3
import math
import os
import subprocess
import json

# 缓存文件，避免重复运行 CACTI 增加编译等待时间
CACHE_FILE = ".cacti_lookup_cache.json"

def run_cacti(size_bytes, ways, block_size=64, tech_node_nm=22):
    """生成临时 CACTI 配置并静默运行"""
    cfg_content = f"""
-size (bytes) {size_bytes}
-block size (bytes) {block_size}
-associativity {ways}
-read-write port 1
-exclusive read port 0
-exclusive write port 0
-single ended read ports 0
-technology (u) {tech_node_nm / 1000.0}
-operating temperature (K) 360
-cache type "cache"
-data array cell type "itrs-hp"
-tag array cell type "itrs-hp"
-access mode "fast"
-design objective weight 0:0:100:0:0
"""
    with open("temp_cacti.cfg", "w") as f:
        f.write(cfg_content)
    
    # 静默执行 cacti
    cmd = "./cacti/cacti -f temp_cacti.cfg"
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    
    # 从 CACTI 输出日志中提取 Access time (ns)
    access_time_ns = 1.0 # 默认保底值
    for line in result.stdout.split("\n"):
        if "Access time (ns):" in line:
            access_time_ns = float(line.split(":")[1].strip())
            break
            
    if os.path.exists("temp_cacti.cfg"):
        os.remove("temp_cacti.cfg")
        
    return access_time_ns

def get_hit_latency(sets, ways, block_size, freq_ghz):
    size_bytes = sets * ways * block_size
    key = f"{size_bytes}_{ways}_{block_size}_{freq_ghz}"
    
    # 1. 尝试读取本地 Cache
    lookup_cache = {}
    if os.path.exists(CACHE_FILE):
        with open(CACHE_FILE, "r") as f:
            lookup_cache = json.load(f)
            
    if key in lookup_cache:
        return lookup_cache[key]
    
    # 2. 调用 CACTI 评估物理延迟 (ns)
    access_time_ns = run_cacti(size_bytes, ways, block_size)
    
    # 3. 换算为 Clock Cycles: Cycles = ceil(Access_Time_ns * Freq_GHz)
    # 留出额外的 Tag 比较与 MUX 传输 Margin，保底最少 1 cycle
    cycles = max(1, math.ceil(access_time_ns * freq_ghz))
    
    # 保存结果到缓存
    lookup_cache[key] = cycles
    with open(CACHE_FILE, "w") as f:
        json.dump(lookup_cache, f, indent=2)
        
    return cycles

if __name__ == "__main__":
    # 示例：根据传入参数计算 L1D 延迟 (512 sets, 8 ways, 64B, 4.0GHz)
    latency = get_hit_latency(sets=512, ways=8, block_size=64, freq_ghz=4.0)
    print(latency)