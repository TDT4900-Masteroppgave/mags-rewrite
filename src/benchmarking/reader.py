import subprocess
import csv
from io import StringIO
from typing import List, Dict
import json
import pandas as pd
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]

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
    
    # retrieve the project root directory (two levels up from this file)
    
    print(f"Running program '{program}' on dataset '{dataset_name}'...")
    output = subprocess.run(
        [f"./build/{program}", f"data/mags_data/small_datasets/{datasets_file}"],
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
