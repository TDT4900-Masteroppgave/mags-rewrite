from typing import List, Dict
import matplotlib.pyplot as plt
import pandas as pd

def plot_grouped_bars(df: pd.DataFrame, y_label:str, title: str, output_path: str = None):
    """
    Creates a grouped bar chart with dataset on X and bars for each program.
    Saves to output_path if provided.
    """
    if df.empty:
        raise ValueError("DataFrame is empty. Nothing to plot.")

    # Pivot: rows=datasets, columns=programs, values=value
    pivot = df.pivot_table(index="dataset", columns="program", values=y_label, aggfunc="mean")
    programs = list(pivot.columns)
    datasets = list(pivot.index)

    n_datasets = len(datasets)
    n_programs = len(programs)

    fig, ax = plt.subplots(figsize=(max(6, n_datasets * 1.2), max(4, 3 + 0.3 * n_programs)))

    x = range(n_datasets)
    total_width = 0.8
    bar_width = total_width / max(1, n_programs)
    offsets = [(-total_width/2 + (i + 0.5)*bar_width) for i in range(n_programs)]

    # Plot each program as a separate bar series
    for i, program_name in enumerate(programs):
        yvals = pivot[program_name].values  # NaNs allowed; matplotlib will skip
        ax.bar(
            [xi + offsets[i] for xi in x],
            yvals,
            width=bar_width,
            label=str(program_name)
        )

    ax.set_title(title)
    ax.set_xlabel("Dataset")
    ax.set_ylabel(y_label)
    ax.set_xticks(list(x))
    ax.set_xticklabels(datasets, rotation=20, ha="right")
    ax.legend(title="Program", loc="best", frameon=False)
    ax.grid(axis="y", linestyle="--", alpha=0.3)

    fig.tight_layout()
    if output_path:
        plt.savefig(output_path, dpi=200, bbox_inches="tight")
        print(f"Saved plot to {output_path}")