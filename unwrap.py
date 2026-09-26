#!/usr/bin/env python3

import sys
import zipfile
import pyzipper
import shutil
from pathlib import Path

START_FILE = "layer_1.zip"   # first layer
MAX_LAYERS = 1337
WORKDIR = Path(".")
FINAL_DIR = Path("extracted_final")

def open_any(path):
    return pyzipper.AESZipFile(path), "pyzipper"

def main():
    current = Path(START_FILE)
    if not current.exists():
        if Path("layers.zip").exists():
            print("[*] Extracting layers.zip ...")
            with zipfile.ZipFile("layers.zip") as z:
                z.extractall(".")
        if not current.exists():
            print(f"[!] {START_FILE} not found — extract layers.zip first", file=sys.stderr)
            sys.exit(1)

    for n in range(1, MAX_LAYERS + 1):
        password = str(n).encode()
        print(f"[{n}/{MAX_LAYERS}] Opening {current.name} with password '{n}' ...", end=" ")

        try:
            zf, backend = open_any(current)
        except Exception as e:
            print(f"ERROR opening archive: {e}")
            sys.exit(1)

        names = zf.namelist()
        if not names:
            print("archive is empty, stopping.")
            break

        # expect one file per layer — the next layer_N.zip or the final file
        next_name = names[0]
        out_dir = Path(f"step_{n}")
        out_dir.mkdir(exist_ok=True)

        try:
            if backend == "pyzipper":
                zf.extract(next_name, path=out_dir, pwd=password)
            else:
                zf.extract(next_name, path=out_dir, pwd=password)
        except RuntimeError as e:
            print(f"WRONG PASSWORD or error: {e}")
            sys.exit(1)
        finally:
            zf.close()

        extracted_path = out_dir / next_name
        print(f"-> {next_name} ({extracted_path.stat().st_size} bytes)")

        # if the next file is not a zip, stop and copy it to the final directory
        is_next_zip = next_name.lower().endswith(".zip")
        if not is_next_zip:
            FINAL_DIR.mkdir(exist_ok=True)
            shutil.copy(extracted_path, FINAL_DIR / next_name)
            print(f"\n[OK] Reached the final file: {next_name}")
            print(f"     Saved to {FINAL_DIR / next_name}")
            return

        current = extracted_path

    print(f"\n[OK] Went through all {MAX_LAYERS} layers. Last file: {current}")

if __name__ == "__main__":
    main()