#!/usr/bin/env python3
"""Package and verify the locally built Photon nRF52 application updates."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import zipfile

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
build = root / '.pio/build/meshsmith_photon_nrf52_e22p_30dbm_batteryinfo_powersaving_repeater'
uf2 = (build / 'firmware.uf2').read_bytes()
assert len(uf2) > 0 and len(uf2) % 512 == 0, 'Invalid UF2 length'
blocks = []
for offset in range(0, len(uf2), 512):
    block = uf2[offset:offset + 512]
    m0, m1, flags, address, size, number, count, family = struct.unpack('<8I', block[:32])
    assert (m0, m1) == (0x0A324655, 0x9E5D5157)
    assert struct.unpack('<I', block[-4:])[0] == 0x0AB16F30
    assert flags & 0x2000 and family == 0xADA52840, 'Wrong MCU family'
    assert 0 < size <= 476 and 0x27000 <= address < address + size <= 0xED000
    assert count == len(uf2) // 512
    blocks.append((number, address, size, block[32:32+size]))
assert sorted(n for n, _, _, _ in blocks) == list(range(len(blocks)))
assert min(a for _, a, _, _ in blocks) == 0x27000, 'Wrong application origin'
with zipfile.ZipFile(build / 'firmware.zip') as archive:
    assert archive.testzip() is None, 'Invalid DFU zip'
    dfu = json.loads(archive.read('manifest.json'))
    application = dfu['manifest']['application']
    app_bin = archive.read(application['bin_file'])
    assert b'v1.18.0-PS18-Photon-nRF-BattPS' in app_bin, 'Wrong firmware version'
    assert b'Meshsmith Photon Repeater' in app_bin, 'Wrong target identity'
    assert b'battery=%.2fv %d%%' in app_bin
    assert b' charge=%+.2f%%/hr' in app_bin and b' temp=%.1fF' in app_bin
    assert b'%.2fv est=%d%% %s' not in app_bin, 'Unexpected battery alert format'
    expected = bytearray(b'\xff' * (max(a+s for _, a, s, _ in blocks) - 0x27000))
    for _, address, size, payload in blocks:
        expected[address - 0x27000:address - 0x27000 + size] = payload
    assert bytes(expected[:len(app_bin)]) == app_bin, 'UF2/DFU firmware mismatch'
    assert all(b == 0xff for b in expected[len(app_bin):]), 'Unexpected UF2 tail'

output = args.output.resolve()
output.mkdir(parents=True, exist_ok=True)
files = {}
for ext in ('uf2', 'zip', 'hex', 'elf'):
    src = build / ('firmware.' + ext)
    dst = output / ('photon-nrf52-repeater-powersaving-v18.' + ext)
    shutil.copy2(src, dst)
    files[dst.name] = {'sha256': hashlib.sha256(dst.read_bytes()).hexdigest(), 'bytes': dst.stat().st_size}
shutil.copy2(root / 'docs/photon-nrf52-powersaving-v18.md', output / 'README.md')
files['README.md'] = {'sha256': hashlib.sha256((output / 'README.md').read_bytes()).hexdigest(),
                      'bytes': (output / 'README.md').stat().st_size}
commit = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()
manifest = {
    'target': 'meshsmith_photon_nrf52_e22p_30dbm_batteryinfo_powersaving_repeater', 'source_commit': commit,
    'source_dirty': bool(subprocess.check_output(['git', 'status', '--porcelain'], cwd=root, text=True).strip()),
    'upstream_commit': '6beb4506dc354b80178fe10477756d8a2ab2ce6f',
    'mcu': 'nRF52840', 'softdevice': 'S140 7.3.0', 'uf2_family': '0xADA52840',
    'application_origin': '0x27000', 'application_end_exclusive': hex(max(a+s for _, a, s, _ in blocks)),
    'uf2_blocks': len(blocks), 'uf2_dfu_match': True,
    'hardware_tested': False, 'files': files,
}
(output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
checksums = {name: meta['sha256'] for name, meta in files.items()}
checksums['manifest.json'] = hashlib.sha256((output / 'manifest.json').read_bytes()).hexdigest()
(output / 'SHA256SUMS.txt').write_text(''.join(f'{digest}  {name}\n' for name, digest in checksums.items()))
print(json.dumps({'output': str(output), 'source_commit': commit, 'source_dirty': manifest['source_dirty'],
                  'uf2_blocks': len(blocks), 'application_end_exclusive': manifest['application_end_exclusive'],
                  'uf2_dfu_match': True, 'hardware_tested': False}, indent=2))
