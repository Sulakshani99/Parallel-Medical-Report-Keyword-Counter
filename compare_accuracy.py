import os
import matplotlib.pyplot as plt

# File paths
result_files = {
    'Serial': 'outputs/result_serial.txt',
    'OpenMP': 'outputs/result_openmp.txt',
    'MPI': 'outputs/result_mpi.txt',
    'Hybrid': 'outputs/result_hybrid.txt',
}

# Read keyword counts from a result file
def read_counts(filepath):
    counts = {}
    with open(filepath, 'r') as f:
        for line in f:
            if ':' in line:
                key, value = line.strip().split(':', 1)
                counts[key.strip()] = int(value.strip())
    return counts

# Read all results
data = {name: read_counts(path) for name, path in result_files.items()}

# Serial is ground truth
serial_counts = data['Serial']
keywords = list(serial_counts.keys())

# Calculate accuracy metrics
def calculate_metrics(reference, test):
    total = len(reference)
    exact_matches = sum(1 for k in reference if reference[k] == test.get(k, None))
    percent_match = (exact_matches / total) * 100
    mae = sum(abs(reference[k] - test.get(k, 0)) for k in reference) / total
    return percent_match, mae

metrics = {}
for name in ['OpenMP', 'MPI', 'Hybrid']:
    percent_match, mae = calculate_metrics(serial_counts, data[name])
    metrics[name] = {'percent_match': percent_match, 'mae': mae}

# Plotting
fig, ax1 = plt.subplots(figsize=(8, 5))

implementations = list(metrics.keys())
percent_matches = [metrics[name]['percent_match'] for name in implementations]
maes = [metrics[name]['mae'] for name in implementations]

bar_width = 0.35
x = range(len(implementations))

# Bar for percent match
ax1.bar(x, percent_matches, width=bar_width, label='Exact Match (%)', color='tab:blue')
ax1.set_ylabel('Exact Match (%)', color='tab:blue')
ax1.set_ylim(0, 105)
ax1.set_xticks(x)
ax1.set_xticklabels(implementations)

# Twin axis for MAE
ax2 = ax1.twinx()
ax2.bar([i + bar_width for i in x], maes, width=bar_width, label='Mean Absolute Error', color='tab:orange')
ax2.set_ylabel('Mean Absolute Error', color='tab:orange')

# Title and legend
plt.title('Accuracy Comparison of Keyword Counting Implementations')
fig.legend(loc='upper right', bbox_to_anchor=(1,1), bbox_transform=ax1.transAxes)
plt.tight_layout()
plt.show() 
