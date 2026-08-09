#!/usr/bin/env bash

set -euo pipefail

if [ -z "${FIRMWARE_VERSION:-}" ]; then
  echo "FIRMWARE_VERSION must be set"
  exit 1
fi

OUTPUT_DIR="${OUTPUT_DIR:-out}"
FIRMWARE_BUILD_DATE="$(date '+%d-%b-%Y')"
BUILD_VERSION="${FIRMWARE_VERSION}-Heltec-V4-BatteryInfo"
ENV_NAME="heltec_v4_batteryinfo_repeater"
ASSET_NAME="Heltec-V4-BatteryInfo-Repeater"

rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}"

build_flags="${PLATFORMIO_BUILD_FLAGS:-}"
build_flags="${build_flags} -DFIRMWARE_BUILD_DATE='\"${FIRMWARE_BUILD_DATE}\"'"
build_flags="${build_flags} -DFIRMWARE_VERSION='\"${BUILD_VERSION}\"'"

echo "Building ${ENV_NAME} -> ${ASSET_NAME}-${FIRMWARE_VERSION}"
rm -rf ".pio/build/${ENV_NAME}"

PLATFORMIO_BUILD_FLAGS="${build_flags}" pio run -e "${ENV_NAME}"
PLATFORMIO_BUILD_FLAGS="${build_flags}" pio run -t mergebin -e "${ENV_NAME}"

cp ".pio/build/${ENV_NAME}/firmware.bin" "${OUTPUT_DIR}/${ASSET_NAME}-${FIRMWARE_VERSION}.bin"
cp ".pio/build/${ENV_NAME}/firmware-merged.bin" "${OUTPUT_DIR}/${ASSET_NAME}-${FIRMWARE_VERSION}-merged.bin"

if [ -f ".pio/build/${ENV_NAME}/partitions.bin" ]; then
  cp ".pio/build/${ENV_NAME}/partitions.bin" "${OUTPUT_DIR}/${ASSET_NAME}-${FIRMWARE_VERSION}-partitions.bin"
fi
if [ -f ".pio/build/${ENV_NAME}/bootloader.bin" ]; then
  cp ".pio/build/${ENV_NAME}/bootloader.bin" "${OUTPUT_DIR}/${ASSET_NAME}-${FIRMWARE_VERSION}-bootloader.bin"
fi

(
  cd "${OUTPUT_DIR}"
  sha256sum ./*.bin > SHA256SUMS.txt
)
printf 'Built files:\n'
printf '  %s\n' "${OUTPUT_DIR}"/*
