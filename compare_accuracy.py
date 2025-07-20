import matplotlib.pyplot as plt
import numpy as np
import re

# --- File Paths ---
PERF_PATH = 'outputs/performance.txt'
RESULT_PATHS = {
    'Serial': 'outputs/result_serial.txt',
    'OpenMP': 'outputs/result_openmp.txt',
    'MPI': 'outputs/result_mpi.txt',
    'Hybrid': 'outputs/result_hybrid.txt',
}


def parse_result_file(path):
    counts = {}
    with open(path, 'r') as f:
        for line in f:
            if ':' in line:
                k, v = line.strip().split(':', 1)
                counts[k.strip()] = int(v.strip())
    return counts

def parse_performance_file(perf_path):
    perf_pattern = re.compile(r"(Serial|OpenMP|MPI|Hybrid) version time: ([0-9.]+) seconds\s+No\. of Process(?:ors|es): (\d+)\s+No\. of Threads: (\d+)", re.I)
    perf_data = []
    with open(perf_path, 'r') as f:
        for line in f:
            m = perf_pattern.search(line)
            if m:
                method, time, procs, threads = m.groups()
                # Build configuration string
                if method == 'Serial':
                    config = '1 core, 1 thread'
                elif method == 'OpenMP':
                    config = f'{threads} threads'
                elif method == 'MPI':
                    config = f'{procs} processes'
                elif method == 'Hybrid':
                    config = f'{procs}x{threads} (MPIxOMP)'
                else:
                    config = ''
                perf_data.append({'method': method, 'config': config, 'time': float(time), 'procs': procs, 'threads': threads})
    # Exclude Hybrid 8x2 (MPIxOMP)
    perf_data = [d for d in perf_data if not (d['method'] == 'Hybrid' and d['config'].startswith('8x2'))]
    return perf_data


def analyze_performance(perf_data):
    # Find the fastest serial time for speedup calculation
    serial_times = [d['time'] for d in perf_data if d['method'].lower() == 'serial']
    serial_time = min(serial_times) if serial_times else 1.0
    for d in perf_data:
        d['speedup'] = serial_time / d['time'] if d['time'] > 0 else 0
    return perf_data, serial_time

def print_performance_table(perf_data):
    print("\n| Method   | Configuration         | Time (s)   | Speedup (vs Serial) |")
    print("|----------|----------------------|------------|---------------------|")
    for d in perf_data:
        print(f"| {d['method']:<8} | {d['config']:<20} | {d['time']:<10.6f} | {d['speedup']:<19.2f}|")

def plot_performance(perf_data):
    labels = [f"{d['method']} ({d['config']})" for d in perf_data]
    times = [d['time'] for d in perf_data]
    speedups = [d['speedup'] for d in perf_data]
    x_perf = range(len(labels))
    # Plot 1: Execution Time
    plt.figure(figsize=(10, 6))
    plt.bar(x_perf, times, color='tab:green')
    plt.ylabel('Execution Time (s)')
    plt.xticks(x_perf, labels, rotation=30, ha='right')
    plt.title('Execution Time of Each Configuration')
    plt.tight_layout(rect=[0, 0, 1, 0.97])
    plt.show()
    # Plot 2: Speedup
    plt.figure(figsize=(10, 6))
    plt.bar(x_perf, speedups, color='tab:blue')
    plt.ylabel('Speedup (vs Serial)')
    plt.xticks(x_perf, labels, rotation=30, ha='right')
    plt.title('Speedup of Each Configuration')
    plt.tight_layout(rect=[0, 0, 1, 0.97])
    plt.show()

def analyze_rmse():
    # Load all result files
    serial_counts = parse_result_file(RESULT_PATHS['Serial'])
    openmp_counts = parse_result_file(RESULT_PATHS['OpenMP'])
    mpi_counts = parse_result_file(RESULT_PATHS['MPI'])
    hybrid_counts = parse_result_file(RESULT_PATHS['Hybrid'])
    methods = ['OpenMP', 'MPI', 'Hybrid']
    results = [openmp_counts, mpi_counts, hybrid_counts]
    rmse_values = []
    serial_vec = np.array([serial_counts[k] for k in serial_counts])
    for res in results:
        vec = np.array([res[k] for k in serial_counts])
        rmse = np.sqrt(np.mean((vec - serial_vec) ** 2))
        rmse_values.append(rmse)
    return methods, rmse_values

def print_rmse_table(methods, rmse_values):
    print("\n| Method   | RMSE (vs Serial) |")
    print("|----------|---------------|")
    for m, r in zip(methods, rmse_values):
        print(f"| {m:<8} | {r:<13.6f}|")

def plot_rmse(methods, rmse_values):
    plt.figure(figsize=(7, 5))
    plt.bar(methods, rmse_values, color=['tab:orange', 'tab:purple', 'tab:gray'])
    plt.ylabel('RMSE (vs Serial)')
    plt.title('Accuracy Comparison (RMSE)')
    plt.tight_layout()
    plt.show()


def main():
    # Performance Analysis
    perf_data = parse_performance_file(PERF_PATH)
    perf_data, _ = analyze_performance(perf_data)
    print_performance_table(perf_data)
    plot_performance(perf_data)
    # RMSE Accuracy Analysis
    methods, rmse_values = analyze_rmse()
    print_rmse_table(methods, rmse_values)
    plot_rmse(methods, rmse_values)

if __name__ == "__main__":
    main() 

