#!/usr/bin/env python3
"""Build and package the release-matched RAK4631 repeater; never flash."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import zipfile

ENV = "RAK_4631_batteryinfo_powersaving_repeater"
PREFIX = "rak4631-v1.17.1-batteryinfo-powersaving-repeater"
ROOT = Path(__file__).resolve().parents[1]


def git(*args):
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if output == ROOT or ROOT in output.parents:
        parser.error("Output must be outside the source worktree")
    output.mkdir(parents=True, exist_ok=True)
    for command in (["pio", "run", "-e", ENV],
                    ["pio", "run", "-e", ENV, "-t", "create_uf2"]):
        subprocess.run(command, cwd=ROOT, check=True)
    build = ROOT / ".pio" / "build" / ENV
    uf2 = (build / "firmware.uf2").read_bytes()
    if not uf2 or len(uf2) % 512:
        raise RuntimeError("Invalid UF2 size")
    blocks = len(uf2) // 512
    addresses = []
    for index in range(blocks):
        block = uf2[index * 512:(index + 1) * 512]
        magic0, magic1, flags, address, length, number, total, family = struct.unpack_from("<8I", block)
        if (magic0, magic1, struct.unpack_from("<I", block, 508)[0]) != (0x0A324655, 0x9E5D5157, 0x0AB16F30):
            raise RuntimeError("Invalid UF2 magic")
        if not flags & 0x2000 or family != 0xADA52840 or number != index or total != blocks:
            raise RuntimeError("Invalid UF2 family/block metadata")
        if length != 256 or address < 0x26000 or address + length > 0xED000:
            raise RuntimeError("UF2 exceeds RAK4631 application boundaries")
        addresses.append(address)
    if min(addresses) != 0x26000 or len(set(addresses)) != blocks:
        raise RuntimeError("Invalid UF2 application origin or duplicate blocks")
    with zipfile.ZipFile(build / "firmware.zip") as archive:
        if archive.testzip() is not None:
            raise RuntimeError("DFU ZIP checksum failure")
        manifest = json.loads(archive.read("manifest.json"))["manifest"]["application"]
        app = archive.read(manifest["bin_file"])
        archive.read(manifest["dat_file"])
        if not app:
            raise RuntimeError("Empty DFU application")
        payload = {address: uf2[i * 512 + 32:i * 512 + 288] for i, address in enumerate(addresses)}
        for offset in range(0, len(app), 256):
            chunk = app[offset:offset + 256]
            if payload.get(0x26000 + offset, b"")[:len(chunk)] != chunk:
                raise RuntimeError("UF2 and DFU application payloads differ")
    files = []
    for extension in ("uf2", "zip", "hex", "elf"):
        target = output / f"{PREFIX}.{extension}"
        shutil.copy2(build / f"firmware.{extension}", target)
        files.append(target)
    provenance = output / "provenance.json"
    provenance.write_text(json.dumps({
        "environment": ENV, "version": "v1.17.1", "source_commit": git("rev-parse", "HEAD"),
        "release_base": git("rev-parse", "repeater-v1.17.1^{commit}"),
        "source_dirty": bool(git("status", "--porcelain")),
        "uf2_family": "0xADA52840", "application_origin": "0x26000",
        "hardware_tested": False,
    }, indent=2) + "\n")
    files.append(provenance)
    checksum_file = output / "SHA256SUMS.txt"
    checksum_file.write_text("".join(
        f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n" for path in files
    ))
    print(f"Validated UF2/DFU payloads and packaged firmware in {output}")


if __name__ == "__main__":
    main()
