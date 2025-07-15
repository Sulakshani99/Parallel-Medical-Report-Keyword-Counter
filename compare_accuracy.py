import matplotlib.pyplot as plt
import re

# --- Performance Analysis Only ---
perf_path = 'outputs/performance.txt'
perf_data = []
perf_pattern = re.compile(r"(Serial|OpenMP|MPI|Hybrid) version time: ([0-9.]+) seconds\s+No\. of Process(?:ors|es): (\d+)\s+No\. of Threads: (\d+)", re.I)

with open(perf_path, 'r') as f:
    for line in f:
        m = perf_pattern.search(line)
        if m:
            method, time, procs, threads = m.groups()
            label = method
            if method == 'OpenMP':
                label += f' ({threads} threads)'
            elif method == 'MPI':
                label += f' ({procs} procs)'
            elif method == 'Hybrid':
                label += f' ({procs}x{threads})'
            perf_data.append({'label': label, 'method': method, 'time': float(time)})

# Find the fastest serial time for speedup calculation
serial_times = [d['time'] for d in perf_data if d['method'].lower() == 'serial']
serial_time = min(serial_times) if serial_times else 1.0
for d in perf_data:
    d['speedup'] = serial_time / d['time'] if d['time'] > 0 else 0

# --- Plotting ---
fig, ax1 = plt.subplots(figsize=(10, 6))

labels = [d['label'] for d in perf_data]
times = [d['time'] for d in perf_data]
speedups = [d['speedup'] for d in perf_data]
x_perf = range(len(labels))

bar_width = 0.35
ax1.bar(x_perf, times, width=bar_width, label='Execution Time (s)', color='tab:green')
ax1.set_ylabel('Execution Time (s)', color='tab:green')
ax1.set_xticks(x_perf)
ax1.set_xticklabels(labels, rotation=30, ha='right')

ax2 = ax1.twinx()
ax2.plot(x_perf, speedups, label='Speedup', color='tab:red', marker='o')
ax2.set_ylabel('Speedup (vs Serial)', color='tab:red')

ax1.set_title('Performance Analysis: Execution Time and Speedup')
fig.tight_layout(rect=[0, 0, 1, 0.97])
plt.show() 
