"""Check the immutable, MIT-licensed SARibbon source imported into the build."""

import hashlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXPECTED = {
    "SARibbon.cpp": "9c844e16af1d9a7ddbe86d18065923457424ea40c2562abe44fabbd8d4068d14",
    "SARibbon.h": "1e7593ce379a13d4801140fcdf891550aa7e9736d71cd34028457aef71241244",
}


def main():
    vendor = ROOT / "libraries" / "saribbon"
    for name, expected in EXPECTED.items():
        actual = hashlib.sha256((vendor / name).read_bytes()).hexdigest()
        if actual != expected:
            raise SystemExit(f"SARibbon upstream artifact changed: {name}: {actual}")
    source_license = (vendor / "LICENSE").read_text(encoding="utf-8")
    shipped_license = (ROOT / "licenses" / "SARibbon-MIT.txt").read_text(encoding="utf-8")
    if source_license != shipped_license or "Copyright (c) 2020 czyt1988" not in source_license:
        raise SystemExit("SARibbon upstream/shipped MIT attribution mismatch")
    print("PASS: SARibbon v2.9.0 byte-exact source and MIT attribution")


if __name__ == "__main__":
    main()
