# Ivy Sim - Procedural ivy generator for Godot 4
# Copyright (C) 2026 Max Heinze
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
# Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
# Distributed WITHOUT ANY WARRANTY; see LICENSE for details.

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
        exe_name = "clang-format.exe" if os.name == "nt" else "clang-format"
        cf_exe = os.path.join(clang_format.__path__[0], "data", "bin", exe_name)
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
