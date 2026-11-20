# Copyright (c) 2026 Max Heinze. All rights reserved.
#
# Licensed for non-commercial use with attribution only.
# Commercial use is strictly prohibited.

"""
Batch code formatter for ivy-sim C++ sources and headers.
Formats all files in src/ and tests/ using clang-format according to .clang-format.
"""

import glob
import os
import subprocess
import sys

def main():
    try:
        import clang_format
        cf_exe = os.path.join(clang_format.__path__[0], "data", "bin", "clang-format.exe")
        if not os.path.isfile(cf_exe):
            cf_exe = "clang-format"
    except ImportError:
        cf_exe = "clang-format"

    root_dir = os.path.dirname(os.path.abspath(__file__))
    patterns = [
        os.path.join(root_dir, "src", "**", "*.cpp"),
        os.path.join(root_dir, "src", "**", "*.h"),
        os.path.join(root_dir, "tests", "**", "*.cpp"),
        os.path.join(root_dir, "tests", "**", "*.h"),
    ]

    files = []
    for pattern in patterns:
        files.extend(glob.glob(pattern, recursive=True))

    # Exclude generated code if any
    files = [f for f in files if "src" + os.sep + "gen" not in f]

    if not files:
        print("No C++ files found to format.")
        return 0

    print(f"Formatting {len(files)} C++ files with clang-format...")
    result = subprocess.run([cf_exe, "-i"] + files)
    if result.returncode == 0:
        print("All files formatted successfully!")
    else:
        print(f"clang-format exited with code {result.returncode}", file=sys.stderr)
    return result.returncode

if __name__ == "__main__":
    sys.exit(main())
