import json
from typing import List, Dict
import pandas as pd
import argparse
from pathlib import Path

from reader import read_program
from plotter import plot_grouped_bars

PROJECT_ROOT = Path(__file__).resolve().parents[2]

def collect(datasets: Dict[str, str], programs: List[str], output_file: str) -> pd.DataFrame:
    results = {}

    for dataset_name, dataset_file in datasets.items():
        if dataset_name not in results:
            results[dataset_name] = []
            
        for program in programs:
            output = read_program(program, dataset_name, dataset_file)
            results[dataset_name].extend(output)
        
        print(f"Completed benchmarking for dataset '{dataset_name}'.")
        
    if output_file:
        with open(PROJECT_ROOT/output_file, "w") as f:
            json.dump(results, f, indent=4)

def collect_df(data: Dict[str, List[Dict[str, str]]], y_label: str) -> pd.DataFrame:
    if not data:
        raise ValueError("Data is empty. Nothing to plot.")

    # Flatten data into a list of rows and extract relevant columns: dataset, program, y_label
    rows = []
    for dataset_name, programs in data.items():
        for program_results in programs:
            filtered_program_results = {k: v for k, v in program_results.items() if k in ["program", y_label]}
            filtered_program_results["dataset"] = dataset_name
            rows.append(filtered_program_results)

    df = pd.DataFrame(rows)
    
     # Convert the y_label column to numeric
    df[y_label] = pd.to_numeric(df[y_label], errors="coerce")
    
    return df

def main(datasets: Dict[str, str], programs: List[str]):
    parser = argparse.ArgumentParser(prog="benchmarking")
    sub = parser.add_subparsers(dest="cmd", required=True)
    results = {}

    p_collect = sub.add_parser("collect", help="Run benchmarks and save results.")
    p_collect.add_argument("--out", required=True)

    p_plot = sub.add_parser("plot", help="Plot from a dataframe.")
    p_plot.add_argument("--title", default="Metric per Dataset by Program")
    p_plot.add_argument("--y", required=True)
    p_plot.add_argument("--input", required=True)
    p_plot.add_argument("--out", required=True)

    args = parser.parse_args()

    # python3 src/benchmarking/cli.py collect --out results/data.json
    if args.cmd == "collect":
        collect(datasets, programs, args.out)
        print(f"Saved results to {args.out}")

    # python3 src/benchmarking/cli.py plot --input results/data.json --y relative_size --out results/plots/relative_size.png --title "Relative Size per Dataset by Program"
    # python3 src/benchmarking/cli.py plot --input results/data.json --y merge --out results/plots/merge_time.png --title "Merge Time per Dataset by Program"
    elif args.cmd == "plot":
        # load data from file
        with open(PROJECT_ROOT/args.input, "r") as f:
            results = json.load(f)
         
        df = collect_df(results, y_label=args.y)
        
        plot_grouped_bars(df, y_label=args.y, title=args.title, output_path=PROJECT_ROOT/args.out)

if __name__ == "__main__":
     # List of datasets to benchmark
        # datasets = {
        #     "email": "email-Eu-core.txt",
        #     "facebook": "facebook_combined.txt",
        # }
    
    datasets = {
        "CA": "as-caida20071105.txt",
        "EN": "Email-Enron.txt",
        "BK": "Brightkite_edges.txt",
        "EA": "Email-EuAll.txt",
        "SL": "Slashdot0902.txt",
        "DB": "com-dblp.ungraph.txt"
    }

    # Program to benchmark
    programs = [
        "mags",
        "mags_rewrite"
        ]
    
    main(datasets, programs)
    
