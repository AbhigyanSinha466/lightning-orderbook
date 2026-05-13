import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def compare_latency(csv_files):
    plt.figure(figsize=(12, 7))
    
    # Calculate a global P99.9 to use as a consistent X-limit for comparison
    all_p999 = []
    
    latency_col = 'latency_cycles'
    
    for csv_file in csv_files:
        if os.path.exists(csv_file):
            df = pd.read_csv(csv_file)
            if not df.empty:
                if latency_col not in df.columns:
                    current_col = df.columns[0]
                else:
                    current_col = latency_col
                all_p999.append(df[current_col].quantile(0.999))
    
    x_limit = max(all_p999) if all_p999 else 2000

    for csv_file in csv_files:
        if not os.path.exists(csv_file):
            print(f"Warning: {csv_file} not found. Skipping.")
            continue

        label = os.path.basename(csv_file).replace('.csv', '').replace('latency_', '')
        df = pd.read_csv(csv_file)
        
        if df.empty:
            print(f"Warning: {csv_file} is empty. Skipping.")
            continue

        if latency_col not in df.columns:
            current_col = df.columns[0]
        else:
            current_col = latency_col
            
        data = df[current_col]
        
        # Filter extreme outliers for better visualization (up to global P99.9)
        filtered_data = data[data <= x_limit]

        # Plot overlapping histograms with 500 bins for granularity
        n, bins, patches = plt.hist(filtered_data, bins=500, alpha=0.5, label=label, edgecolor=None)
        color = patches[0].get_facecolor()

        # Add median vertical line
        median = data.median()
        plt.axvline(median, color=color, linestyle='--', linewidth=1.5, label=f'{label} Median: {median:.1f} cycles')
        
        print(f"Stats for {label}:")
        print(f"  P50: {data.median():.2f} cycles")
        print(f"  P90: {data.quantile(0.90):.2f} cycles")
        print(f"  P99: {data.quantile(0.99):.2f} cycles")
        print(f"  Avg: {data.mean():.2f} cycles")

    plt.title('OrderBook Latency Distribution Comparison (Cycles)')
    plt.xlabel('Latency (CPU cycles)')
    plt.ylabel('Frequency')
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.legend()
    
    output_png = 'comparison_results.png'
    plt.savefig(output_png)
    print(f"\nComparison plot saved to {output_png}")
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 scripts/compare_impls.py <csv1> <csv2> ...")
        # Default fallback
        files = ["latency_list.csv", "latency_vector.csv"]
    else:
        files = sys.argv[1:]
    
    compare_latency(files)
