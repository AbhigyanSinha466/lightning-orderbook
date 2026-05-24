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

    # Filter outliers (optional, e.g., keep 99.9th percentile to keep plot readable)
    latency_col = 'latency_cycles'
    if latency_col not in df.columns:
        # Fallback if the CSV has a different header or no header
        latency_col = df.columns[0]

    p999 = df[latency_col].quantile(0.999)
    data = df[df[latency_col] <= p999][latency_col]

    plt.figure(figsize=(10, 6))
    # Increased bins to provide a much more granular view of the distribution.
    plt.hist(data, bins=500, color='skyblue', edgecolor='skyblue', alpha=0.5)

    plt.title('Latency Distribution (Frequency vs Cycles)')
    plt.xlabel('Latency (CPU cycles)')
    plt.ylabel('Frequency')
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    # Add percentile vertical lines
    mean = df[latency_col].mean()
    median = df[latency_col].median()
    plt.axvline(mean, color='red', linestyle='dashed', linewidth=1, label=f'Mean: {mean:.1f} cycles')
    plt.axvline(median, color='green', linestyle='dashed', linewidth=1, label=f'Median: {median:.1f} cycles')

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