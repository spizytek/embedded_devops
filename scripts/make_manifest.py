"""Generate a manifest file for the built firmware."""
from __future__ import annotations
from datetime import datetime, timezone
from pathlib import Path
import argparse

import subprocess
import os #to find stm32cubeide path
import shutil #to check if stm32cubeide is installed
import sys

import json
import re
import hashlib

DEFAULT_PROJECT = "ci_cd_demo"
DEFAULT_CONFIG = "Release"
# (EE_FW_[A-Z0-9_]+) is the capture group that matches the macro name, which starts with EE_FW_ 
# "([^"]*)" is the capture group that matches the value of the macro, which is enclosed in double quotes.
VERSION_DEFINE_RE = re.compile(r'^#define\s+(EE_FW_[A-Z0-9_]+)\s+"([^"]*)"$')


# parents[0] = /Users/user/Developer/embedded/stm32/CI_CD/scripts
# parents[1] = /Users/user/Developer/embedded/stm32/CI_CD
def repo_root() -> Path:
    """Get the root directory of the repository."""
    return Path(__file__).resolve().parents[1]

def parse_version_header(version_header_path: Path) -> dict[str, str]:
    """Parse the version.h and extract the version information into a dictionary."""
    version_info: dict[str, str] = {}

    for line in version_header_path.read_text(encoding="utf-8").splitlines():        
        match = VERSION_DEFINE_RE.match(line.strip())
        if match:
            key, value = match.groups()
            version_info[key] = value
    return version_info

def sha256_checksum(file_path: Path) -> str:
    """Calculate the SHA-256 checksum of a file without loading the entire file into memory.."""
    sha256 = hashlib.sha256()

    with file_path.open("rb") as f:
        # Read the file in chunks of 64 KiB to avoid loading the entire file into memory. 
        # The iter() function is used to create an iterator that reads the file in chunks of 65536 bytes (64 KiB) 
        # until the end of the file is reached (indicated by an empty byte string b"").
        for chunk in iter(lambda: f.read(65536), b""): #65,536 bytes = 64 KiB
            #Add each chunk to the hash 
            sha256.update(chunk)

    return sha256.hexdigest()

def collect_artifacts(root:Path, artifact_dir:Path, project_name:str) -> list[dict[str, object]]:
    """Collect the firmaware build artifacts eg. .bin, .elf, .map, .list"""
    artifacts: list[dict[str, object]] = []

    for artifact_extension in (".bin", ".elf", ".map", ".list"):
        # construct an artifact:e.g. ci_cd_demo/Debug/application.bin
        artifact = artifact_dir/f"{project_name}{artifact_extension}"

        if artifact.exists():
            artifacts.append({
                "file":artifact.name,
                "relative_path": str(artifact.relative_to(root) ).replace("\\", "/"),
                "size_bytes": artifact.stat().st_size,
                "sha256": sha256_checksum(artifact),
            })

    return artifacts

def get_arguments() -> argparse.Namespace:
    """Parse argument, derive build identity fields, and write the version.h file."""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument("--project", default=DEFAULT_PROJECT, help="STM32CubeIDE project folder name")
    parser.add_argument("--config", default=DEFAULT_CONFIG, help="Build configuration (e.g., Debug, Release)")

    parser.add_argument("--version-header", type=Path,  help="Optional override for the generated version header path")
    parser.add_argument("--artifact-dir", type=Path,  help="Optional override for the build artifact directory")
    parser.add_argument("--output", type=Path, help="Optional override for the manifest output path")

    return parser.parse_args()

def main()->int:
    """Main function to parse input, gather build evidence and generate json manifest """
    #  get command line arguments
    args = get_arguments()
    # get the root of the repository and git commit hash
    root = repo_root()
    version_header = args.version_header or root/args.project/"Inc"/"version.h"
    artifact_dir = args.artifact_dir or root/args.project/args.config
    output_path  = args.output or root/"out"/"manifest.json"

    # parse version header
    version_values = parse_version_header(version_header)
    artifacts = collect_artifacts(root=root, 
                                  artifact_dir=artifact_dir, 
                                  project_name=args.project)
    

    manifest = {
        "manifest_version": 1,
        "generated_utc": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "project": version_values.get("EE_FW_PROJECT_NAME", args.project),
        "version": version_values.get("EE_FW_VERSION_STRING", "UNKNOWN"),
        "build_config":  version_values.get("EE_FW_BUILD_CONFIG",   args.config),
        "build_id": version_values.get("EE_FW_BUILD_ID", "UNKNOWN"),
        "build_timestamp_utc": version_values.get("EE_FW_BUILD_TIMESTAMP_UTC", "UNKNOWN"),
        "git_sha": version_values.get("EE_FW_GIT_SHA", "UNKNOWN"),
        "artifact_dir": str(artifact_dir.relative_to(root)).replace("\\", "/"),
        "artifact": artifacts,


        "uart_health_line": (
            "HEALTH",
            f"project = {version_values.get("EE_FW_PROJECT_NAME", args.project)}",
            f"version = {version_values.get("EE_FW_VERSION_STRING", "UNKNOWN")}",
            f"build_id = {version_values.get("EE_FW_BUILD_ID", "UNKNOWN")} ",
            f"config = {version_values.get("EE_FW_BUILD_CONFIG",   args.config)}",
            f"git_sha = {version_values.get("EE_FW_GIT_SHA", "UNKNOWN")}",
            f"timestamp = {version_values.get("EE_FW_BUILD_TIMESTAMP_UTC", "UNKNOWN")}",
        ) 

    }

    

    output_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n" , encoding="utf-8")
    print(f"Generated..{output_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

