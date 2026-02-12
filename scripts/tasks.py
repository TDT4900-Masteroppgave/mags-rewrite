import gzip
import urllib.request
import shlex
import shutil
import platform
import sys
import os
from invoke import task
from pathlib import Path

# Standardization: Project Root is one level up from the scripts folder
ROOT = Path(__file__).resolve().parents[1]
EXTERNAL_DIR = ROOT / "external" / "mags-release"

def safe_path(path):
    """
    Cross-platform path quoting.
    - Windows: Uses double quotes (") because cmd.exe doesn't support single quotes.
    - Linux/Mac: Uses shlex.quote (single quotes) for POSIX safety.
    """
    path_str = str(path)
    if platform.system() == "Windows":
        return f'"{path_str}"'
    return shlex.quote(path_str)

@task
def setup(c):
    """Clone original repo and prepare directories."""
    dirs = [ROOT / "data/small", ROOT / "data/large", ROOT / "results/plots"]
    for d in dirs:
        d.mkdir(parents=True, exist_ok=True)

    if not EXTERNAL_DIR.exists():
        print("⬇️ Cloning original MAGS...")
        c.run(f"git clone https://github.com/nedchu/mags-release.git {safe_path(EXTERNAL_DIR)}")

@task
def build_external(c, mode="Release"):
    """Patch and Build the original MAGS code with OpenMP support."""
    # 1. Patch the typo
    pgsum_path = EXTERNAL_DIR / "src" / "pgsum.cpp"
    if pgsum_path.exists():
        content = pgsum_path.read_text()
        if "pragma omp barier" in content:
            print("🩹 Patching 'barier' typo in original code...")
            pgsum_path.write_text(content.replace("pragma omp barier", "pragma omp barrier"))

    # 2. GENERATE CMakeLists.txt dynamically
    # FIX: We now strictly separate MSVC (LLVM OpenMP) from Standard (Classic OpenMP)
    cmake_content = """
cmake_minimum_required(VERSION 3.10)
project(original_mags LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Define source files
set(SOURCES
    src/util.cpp
    src/graph.cpp
    src/gsum.cpp
    src/pgsum.cpp
    run/run_mags.cpp
)

add_executable(mags ${SOURCES})
target_include_directories(mags PRIVATE src src/parallel_hashmap)

if(MSVC)
    # MSVC Case:
    # We DO NOT use find_package(OpenMP) or link OpenMP::OpenMP_CXX here.
    # Linking OpenMP::OpenMP_CXX forces the '/openmp' flag (Classic OpenMP 2.0),
    # which breaks the modern 'declare reduction' directives.
    # Instead, we pass /openmp:llvm directly, which automatically links libomp.lib.
    target_compile_options(mags PRIVATE /W0 /openmp:llvm)
else()
    # Linux/macOS Case:
    # Use standard CMake detection which works correctly for GCC/Clang.
    find_package(OpenMP REQUIRED)
    target_link_libraries(mags PRIVATE OpenMP::OpenMP_CXX)
    target_compile_options(mags PRIVATE -w)
endif()
"""
    target_cmake = EXTERNAL_DIR / "CMakeLists.txt"
    # Always write to ensure the fix is applied
    print("📝 Generating CMakeLists.txt for external repo...")
    target_cmake.write_text(cmake_content)

    # 3. Define Flags for macOS
    extra_flags = ""
    if platform.system() == "Darwin":
        omp_prefix = "/opt/homebrew/opt/libomp"
        if Path(omp_prefix).exists():
            extra_flags = (
                f' -DOpenMP_CXX_FLAGS="-Xpreprocessor -fopenmp -I{omp_prefix}/include"'
                f' -DOpenMP_CXX_LIB_NAMES="omp"'
                f' -DOpenMP_omp_LIBRARY="{omp_prefix}/lib/libomp.dylib"'
            )
        extra_flags += ' -DCMAKE_CXX_COMPILER=clang++'

    # 4. Build
    print(f"⚙️  Configuring External MAGS ({mode})...")
    ext_build = EXTERNAL_DIR / "build"

    cmd_config = f'cmake -S {safe_path(EXTERNAL_DIR)} -B {safe_path(ext_build)} -DCMAKE_BUILD_TYPE={mode}{extra_flags}'
    c.run(cmd_config)

    print(f"🔨 Compiling External MAGS...")
    c.run(f'cmake --build {safe_path(ext_build)} -j')

    # 5. COPY Executable to main build folder
    exe_name = "mags.exe" if platform.system() == "Windows" else "mags"
    src_exe = ext_build / exe_name
    dst_exe = ROOT / "build" / exe_name

    dst_exe.parent.mkdir(parents=True, exist_ok=True)

    if src_exe.exists():
        shutil.copy(src_exe, dst_exe)
        print(f"✅ Copied original binary to {dst_exe}")
    else:
        print(f"⚠️  Warning: Could not find compiled executable at {src_exe}")

@task
def benchmark(c, group="small"):
    """Run benchmark."""
    print(f"🚀 Running {group} benchmarks...")

    cli_path = safe_path(ROOT / "benchmarking" / "cli.py")
    python_exe = safe_path(sys.executable)

    # Run the collection script
    c.run(f"{python_exe} {cli_path} collect --group {group} --out results/data.json",
          pty=(platform.system() != "Windows"))

@task
def plot(c):
    """Generate plots."""
    cli_path = safe_path(ROOT / "benchmarking" / "cli.py")
    python_exe = safe_path(sys.executable)
    use_pty = platform.system() != "Windows"

    c.run(f"{python_exe} {cli_path} plot --input results/data.json --y relative_size --out results/plots/relative_size.png", pty=use_pty)
    c.run(f"{python_exe} {cli_path} plot --input results/data.json --y encoding --out results/plots/encoding_time.png", pty=use_pty)

@task
def data(c):
    """Download SNAP datasets used in the MAGS paper."""
    datasets = {
        "small": [
            ("https://snap.stanford.edu/data/as-caida20071105.txt.gz", "as-caida20071105.txt"),
            ("https://snap.stanford.edu/data/email-Enron.txt.gz", "Email-Enron.txt"),
            ("https://snap.stanford.edu/data/loc-brightkite_edges.txt.gz", "Brightkite_edges.txt"),
            ("https://snap.stanford.edu/data/email-EuAll.txt.gz", "Email-EuAll.txt"),
            ("https://snap.stanford.edu/data/soc-Slashdot0902.txt.gz", "Slashdot0902.txt"),
            ("https://snap.stanford.edu/data/bigdata/communities/com-dblp.ungraph.txt.gz", "com-dblp.ungraph.txt")
        ],
        "large": [
            ("https://snap.stanford.edu/data/amazon0601.txt.gz", "amazon0601.txt"),
            ("https://snap.stanford.edu/data/bigdata/communities/com-youtube.ungraph.txt.gz", "com-youtube.ungraph.txt"),
            ("https://snap.stanford.edu/data/as-skitter.txt.gz", "as-skitter.txt"),
            ("https://snap.stanford.edu/data/com-lj.ungraph.txt.gz", "com-lj.ungraph.txt")
        ]
    }

    for category, files in datasets.items():
        target_dir = ROOT / "data" / category
        target_dir.mkdir(parents=True, exist_ok=True)

        for url, output_name in files:
            dest_path = target_dir / output_name
            if dest_path.exists():
                print(f"[SKIP] {output_name} already exists.")
                continue

            print(f"[DOWNLOADING] {output_name} from SNAP...")
            try:
                with urllib.request.urlopen(url) as response:
                    with gzip.GzipFile(fileobj=response) as uncompressed:
                        with open(dest_path, 'wb') as out_file:
                            shutil.copyfileobj(uncompressed, out_file)
                print(f"[SUCCESS] Saved to {dest_path}")
            except Exception as e:
                print(f"[ERROR] Failed to download {output_name}: {e}")
                if dest_path.exists():
                    dest_path.unlink()

@task
def clean(c):
    """Clean up build artifacts and temporary files."""
    print("🧹 Cleaning up project...")

    targets = [
        ROOT / "build",
        ROOT / "external",
        ROOT / "results",
        ROOT / "data"
    ]

    for path in targets:
        if path.exists():
            if path.is_dir():
                shutil.rmtree(path)
                print(f"   - Removed directory: {path.name}")
            else:
                path.unlink()
                print(f"   - Removed file: {path.name}")
        else:
            print(f"   - Skipped (not found): {path.name}")

    print("✨ Clean complete.")