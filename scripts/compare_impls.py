import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def compare_latency(csv_files):
    plt.figure(figsize=(12, 7))
    
    for csv_file in csv_files:
        if not os.path.exists(csv_file):
            print(f"Warning: {csv_file} not found. Skipping.")
            continue

        label = os.path.basename(csv_file).replace('.csv', '').replace('latency_', '')
        df = pd.read_csv(csv_file)
        
        if df.empty:
            print(f"Warning: {csv_file} is empty. Skipping.")
            continue

        data = df['latency_ns']
        
        # Filter extreme outliers for better visualization (e.g., above P99.9)
        if data.empty:
            print(f"Warning: {csv_file} is empty. Skipping.")
            continue

        # Filter data to only include up to 1500ns for the plot
        filtered_data = data[data <= 1500]

        # Calculate bins covering exactly 0 to 1500ns
        bin_edges = np.linspace(0, 1500, 300)

        # Plot overlapping histograms (rectangles with no space)
        n, bins, patches = plt.hist(filtered_data, bins=bin_edges, alpha=0.5, label=label,
         edgecolor=None, linewidth=0)
        color = patches[0].get_facecolor()

        # Add median line for this implementation
        median = data.median()
        plt.axvline(median, color=color, linestyle='--', linewidth=1.5, label=f'{label} Median: {median:.1f}ns')
        
        print(f"Stats for {label}:")
        print(f"  P50: {data.median():.2f} ns")
        print(f"  P90: {data.quantile(0.90):.2f} ns")
        print(f"  P99: {data.quantile(0.99):.2f} ns")
        print(f"  Avg: {data.mean():.2f} ns")

    plt.title('OrderBook Latency Distribution (Nanoseconds)')
    plt.xlabel('Latency (ns)')
    plt.xlim(left=0)
    plt.ylabel('Frequency')
    plt.grid(True, which="both", ls="-", alpha=0.5)
    plt.legend()
    
    output_png = 'comparison_results.png'
    plt.savefig(output_png)
    print(f"\nComparison plot (histogram) saved to {output_png}")
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 scripts/compare_impls.py <csv1> <csv2> ...")
        # Default fallback
        files = ["latency_list.csv", "latency_vector.csv"]
    else:
        files = sys.argv[1:]
    
    compare_latency(files)
