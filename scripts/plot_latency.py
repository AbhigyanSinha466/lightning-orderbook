import pandas as pd
import matplotlib.pyplot as plt
import sys
import os

def plot_latency(csv_file):
    if not os.path.exists(csv_file):
        print(f"Error: {csv_file} not found.")
        return

    # Load data
    df = pd.read_csv(csv_file)
    
    if df.empty:
        print("Error: CSV file is empty.")
        return

    # Filter outliers (optional, e.g., keep 99th percentile to keep plot readable)
    p99 = df['latency_ns'].quantile(0.99)
    data = df[df['latency_ns'] <= p99]['latency_ns']

    plt.figure(figsize=(10, 6))
    plt.hist(data, bins=100, color='skyblue', edgecolor='black', alpha=0.7)
    
    plt.title('Latency Distribution (Frequency vs Latency)')
    plt.xlabel('Latency (nanoseconds)')
    plt.ylabel('Frequency')
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    # Add percentile vertical lines
    mean = df['latency_ns'].mean()
    median = df['latency_ns'].median()
    plt.axvline(mean, color='red', linestyle='dashed', linewidth=1, label=f'Mean: {mean:.1f}ns')
    plt.axvline(median, color='green', linestyle='dashed', linewidth=1, label=f'Median: {median:.1f}ns')
    
    plt.legend()
    
    output_png = 'latency_distribution.png'
    plt.savefig(output_png)
    print(f"Plot saved to {output_png}")
    plt.show()

if __name__ == "__main__":
    csv_input = "latency_results.csv"
    if len(sys.argv) > 1:
        csv_input = sys.argv[1]
    plot_latency(csv_input)
