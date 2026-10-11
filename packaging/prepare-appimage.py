#!/usr/bin/env python3
"""Fetch verified AppImage inputs and prepare an SDK in a disposable build directory."""
import concurrent.futures
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import urllib.request

work = Path(sys.argv[1]).resolve()
inputs = work / "inputs"
inputs.mkdir(parents=True, exist_ok=True)
manifest = json.loads(Path(__file__).with_name("appimage-inputs.json").read_text())


def fetch(item):
    dest = inputs / item["name"]
    if not dest.exists():
        temporary = dest.with_name(dest.name + ".download")
        with urllib.request.urlopen(item["url"], timeout=120) as source, temporary.open("wb") as output:
            shutil.copyfileobj(source, output)
        temporary.rename(dest)
    digest = hashlib.file_digest(dest.open("rb"), "sha256").hexdigest()
    if digest != item["sha256"]:
        raise RuntimeError(f"SHA-256 mismatch: {dest}; do not execute it or update the pin blindly")
    print("Verified", dest.name, flush=True)


with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
    list(pool.map(fetch, manifest))

qt = work / "Qt"
notices = work / "notices"
tools = work / "tools"
for directory in (qt, notices, tools):
    directory.mkdir(exist_ok=False)

for item in manifest:
    archive = inputs / item["name"]
    if archive.suffix == ".7z":
        target = qt / "lib" if archive.name.startswith("icu-") else qt
        target.mkdir(exist_ok=True)
        subprocess.run(["bsdtar", "-xf", str(archive), "-C", str(target)], check=True)
    elif archive.name.endswith(".AppImage"):
        archive.chmod(0o755)
        target = tools / archive.name.removesuffix("-x86_64.AppImage")
        target.mkdir()
        subprocess.run([str(archive), "--appimage-extract"], cwd=target,
                       check=True, stdout=subprocess.DEVNULL)
    elif archive.name.startswith("qt") and archive.name.endswith(".tar.xz"):
        with tarfile.open(archive) as source:
            for member in source:
                if not member.isfile():
                    continue
                if "/LICENSES/" not in member.name and not member.name.endswith(
                        ("qt_attribution.json", "LICENSE", "LICENSE.txt", "COPYING", "COPYRIGHT")):
                    continue
                target = notices / member.name
                if not target.resolve().is_relative_to(notices.resolve()):
                    raise RuntimeError("Invalid notice path")
                target.parent.mkdir(parents=True, exist_ok=True)
                with source.extractfile(member) as stream:
                    target.write_bytes(stream.read())
    elif archive.name.endswith("LICENSE"):
        shutil.copy2(archive, notices / archive.name)

(qt / "bin/qt.conf").write_text("[Paths]\nPrefix=..\n")
print(f"Prepared {work}; run packaging/build-appimage.sh {work} from the source checkout")
