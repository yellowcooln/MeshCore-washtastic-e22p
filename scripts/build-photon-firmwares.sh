#!/usr/bin/env bash
# Build the requested combo target; package without changing the bootloader.
set -euo pipefail
cd "$(dirname "$0")/.."
: "${OUTPUT_DIR:?Set OUTPUT_DIR to a new artifact directory outside the checkout}"
pio run -e meshsmith_photon_nrf52_e22p_30dbm_batteryinfo_powersaving_repeater
pio run -e meshsmith_photon_nrf52_e22p_30dbm_batteryinfo_powersaving_repeater -t create_uf2
python3 scripts/package_photon_nrf52.py --output "$OUTPUT_DIR"
