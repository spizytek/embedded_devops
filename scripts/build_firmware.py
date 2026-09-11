""" Build firmware script. """
from __future__ import annotations
from datetime import datetime, timezone
from pathlib import Path
import argparse
import subprocess
import os #to find stm32cubeide path
import shutil #to check if stm32cubeide is installed
import sys


DEFAULT_PROJECT = "ci_cd_demo"
DEFAULT_CONFIG = "Release"

# parents[0] = /Users/user/Developer/embedded/stm32/CI_CD/scripts
# parents[1] = /Users/user/Developer/embedded/stm32/CI_CD
def repo_root() -> Path:
    """Get the root directory of the repository."""
    return Path(__file__).resolve().parents[1]

def script_dir() -> Path:
    """Get the directory path of the script."""
    return Path(__file__).resolve().parent

def find_stm32cubeide(explicit_path: Path | None) -> Path:
    """Find the path to the STM32CubeIDE executable."""

    if explicit_path is not None:
        if explicit_path.exists():
            return explicit_path
        else:
            print(f"STM32CubeIDE executable not found at {explicit_path}. Searching in system PATH and common locations...")

    #A list that would store the paths to the stm32cubeide executable if found
    path_list: list[Path] = [] 

    # Check environment variables that might point to STM32CubeIDE installation path
    for env_name in ("STM32CUBEIDE_EXECUTABLE", "STM32CUBEIDE_EXE", "STM32CUBEIDE_DMG", "STM32CUBEIDE_APP", "STM32CUBEIDE_PKG", "STM32CUBEIDE_HOME", "STM32CUBEIDE_HEADLESS", "STM32CUBE_HEADLESS"):
        env_path = os.environ.get(env_name)
        if env_path:
            path_list.append(Path(env_path))

    # Check if STM32CubeIDE is in the system PATH/shell environment
    for executable_name in ("stm32cubeide", "stm32cubeide.exe", "stm32cubeidec", "stm32cubeidec.exe", "stm32cubeide.dmg", "stm32cubeide.app", "stm32cubeide.pkg"):
        resolved = shutil.which(executable_name)
        if resolved:
            path_list.append(Path(resolved))


    path_list.append(Path("/Applications/STM32CubeIDE.app/Contents/MacOS/stm32cubeide"))  # macOS
    path_list.append(Path("/usr/local/bin/stm32cubeide")) # Linux

    # glob() searches for files matching the pattern:"STM32CubeIDE_*/STM32CubeIDE/stm32cubeide.exe" in the "C:/ST" directory and returns a list of matching paths. 
    # The sorted() function is used to sort the list of paths in ascending order. 
    # The extend() method is then used to add the sorted list of paths to the path_list.
    path_list.extend(sorted(Path("C:/ST").glob("STM32CubeIDE_*/STM32CubeIDE/stm32cubeide.exe")))  # Windows
    path_list.extend(sorted(Path("C:/ST").glob("STM32CubeIDE_*/STM32CubeIDE/stm32cubeidec.exe")))  # Windows


    for path in path_list:
        if path.exists():
            return path
    
    raise FileNotFoundError("STM32CubeIDE executable not found. Please ensure it is installed and in your PATH.")


def append_log_line(handle, message:str) -> int:
    """Append a line to the log file."""
    handle.write(f"{message}\n")
    handle.flush()  # Ensure the message is written to the file immediately


def run_step(command: list[str], *, cwd: Path, log_handle)->None:
    """Run a command and log it's output to the log file."""
    append_log_line(log_handle, f"$ {" ".join(command)}")

    result = subprocess.run(
        command,
        cwd=cwd,
        # capture_output=True,
        stdout=log_handle, #manually added to capture stdout to the log file (errors in code)
        stderr=subprocess.STDOUT, #manually added to capture stderr to the log file (errors in code)
        text=True,
        check=False
    )

    if result.stdout:
        append_log_line(log_handle, result.stdout.rstrip())
    if result.stderr:
        append_log_line(log_handle, result.stderr.rstrip())


    append_log_line(log_handle, f"Return code: {result.returncode}")
    append_log_line(log_handle, "-" * 100)  # Separator for clarity

    return result.returncode





def main() -> int:
    """Main function to build the firmware."""

    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument("--project", default=DEFAULT_PROJECT, help="STM32CubeIDE project folder name")
    parser.add_argument("--config", default=DEFAULT_CONFIG, help="Build configuration (e.g., Debug, Release)")
    parser.add_argument("--cubeide", type=Path, help="Optional path to the STM32CubeIDE executable. If not provided, the script will attempt to find it automatically.")

    args =  parser.parse_args()

    # Path
    root = repo_root() # root directory of the repository
    local_scripts_dir = script_dir() # directory of the current script
    project_dir = root / args.project # project directory

    # Logs
    logs_dir = root /"out"/"logs"
    logs_dir.mkdir(parents=True, exist_ok=True)
    logs_path = logs_dir/"build.log"

    # Headless workspace is the workspace directory used by STM32CubeIDE when running in headless mode (without GUI).
    headless_workspace = root/"out"/"headless_workspace" 

    # removing the headless_workspace directory if it exists to ensure a clean build environment
    if headless_workspace.exists():
        shutil.rmtree(headless_workspace)

    # headless_workspace.mkdir(parents=True, exist_ok=True)


    with logs_path.open("w", encoding="utf-8") as log_handle:
        append_log_line(log_handle, f"Building started @utc: {datetime.now(timezone.utc).strftime('%Y-%m-%d %H:%M:%SZ')}")

        append_log_line(log_handle, f"Project: {args.project}")
        append_log_line(log_handle, f"Config: {args.config}")
        append_log_line(log_handle, "")


        version_command = [
            sys.executable,  # Use the current Python interpreter
            str(local_scripts_dir / "gen_version.py"),
            "--project", args.project,
            "--config", args.config,
        ]

        if run_step(version_command, cwd=root, log_handle=log_handle) != 0:
            append_log_line(log_handle, "Version generation failed.")
            # Exit with a non-zero code to indicate failure, firmware build should not proceed if version generation fails
            return 1

        # Find the STM32CubeIDE executable
        try:
            cubeide = find_stm32cubeide(args.cubeide)
        except FileNotFoundError as e:
            append_log_line(log_handle, str(e))
            return 1
         
        build_command = [
            str(cubeide),
            "-nosplash",
            "-application", 
            "org.eclipse.cdt.managedbuilder.core.headlessbuild",
            "-data", str(headless_workspace),  #The -data option tells Eclipse/CubeIDE: "Use this directory as your workspace.
            "-import", str(project_dir),
            "-cleanBuild", f"{args.project}/{args.config}",
        ]


        if run_step(build_command, cwd=root, log_handle=log_handle) != 0:
            append_log_line(log_handle, "Firmware build failed.")
            return 1

        manifest_command = [
            sys.executable, # Use the current Python interpreter
            str(local_scripts_dir / "make_manifest.py"),
            "--project", args.project,
            "--config", args.config,
        ]

        if run_step(manifest_command, cwd=root, log_handle=log_handle) != 0:
            append_log_line(log_handle, "Firmware build failed.")
            return 1
        
        append_log_line(log_handle, f"Build completed successfully @utc: {datetime.now(timezone.utc).strftime('%Y-%m-%d %H:%M:%SZ')}")

    print(f"Build log saved to: {logs_path}")
    return 0




if __name__ == "__main__":
    # print(find_stm32cubeide(None))
    raise SystemExit(main())