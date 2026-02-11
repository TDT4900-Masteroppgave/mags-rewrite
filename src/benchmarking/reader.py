import subprocess
import csv
from io import StringIO
from typing import List, Dict
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]

def get_cleaned_dataset_path(raw_filename: str) -> Path:
    """
    Reads a dataset, cleans it (Undirected, No self-loops, Unique),
    and returns the path to the cached clean file.
    """
    # 1. Locate the Raw File
    potential_paths = [
        PROJECT_ROOT / "data" / "mags_data" / "small_datasets" / raw_filename,
        PROJECT_ROOT / "data" / "test_data" / raw_filename
    ]

    raw_path = None
    for p in potential_paths:
        if p.exists():
            raw_path = p
            break

    if raw_path is None:
        raise FileNotFoundError(f"Could not find dataset '{raw_filename}' in known data directories.")

    # 2. Check for Cached Clean File
    clean_dir = PROJECT_ROOT / "data" / "cleaned"
    clean_dir.mkdir(parents=True, exist_ok=True)
    clean_path = clean_dir / raw_filename

    if clean_path.exists():
        # print(f"Using cached clean dataset: {clean_path}") # Optional logging
        return clean_path

    print(f"Preprocessing dataset: {raw_filename} -> {clean_path}")

    # 3. Clean and Write
    unique_edges = set()
    with open(raw_path, 'r') as f:
        for line in f:
            if line.startswith(('#', '%')): continue

            parts = line.strip().split()
            if len(parts) < 2: continue

            try:
                u, v = int(parts[0]), int(parts[1])
            except ValueError:
                continue

            if u == v: continue # No Self-loops

            if u > v: u, v = v, u # Undirected Canonical Order

            unique_edges.add((u, v))

    with open(clean_path, 'w') as f:
        for u, v in sorted(unique_edges):
            f.write(f"{u} {v}\n")

    return clean_path

def _strip_dict(d: Dict[str, str]) -> Dict[str, str]:
    """Strip whitespace from keys and values."""
    return { (k.strip() if isinstance(k, str) else k):
             (v.strip() if isinstance(v, str) else v)
             for k, v in d.items() }

def parse_dict_stdout(output: str) -> List[Dict[str, str]]:
    """Parse CSV text (with header) to list of dicts and strip whitespace."""
    reader = csv.DictReader(StringIO(output))
    return [_strip_dict(row) for row in reader]

def read_program(program: str, dataset_name: str, datasets_file: str, save: bool = False) -> List[Dict[str, str]]:
    """Run program and return parsed/cleaned rows (list of dicts)."""

    dataset_path = get_cleaned_dataset_path(datasets_file)

    # retrieve the project root directory (two levels up from this file)
    
    print(f"Running program '{program}' on dataset '{dataset_name}'...")
    output = subprocess.run(
        [f"./build/{program}", str(dataset_path)],
        # [f"./build/{program}", f"data/test_data/{datasets_file}"],
        capture_output=True,
        cwd=PROJECT_ROOT, # run the child process from the project root
        text=True,
        check=False
    )
    print(f"Finished running program '{program}' on dataset '{dataset_name}'...")
    
    if output.returncode != 0:
        raise RuntimeError(
            f"{program} failed (exit {output.returncode}) for dataset '{dataset_name}'.\n"
            f"STDERR:\n{output.stderr}"
        )
     
    return parse_dict_stdout(output.stdout)
