#!/usr/bin/env bash
set -euo pipefail

export IDF_PATH="${IDF_PATH:-/opt/esp/esp-idf}"
export ESP_MATTER_PATH="${ESP_MATTER_PATH:-/opt/esp/esp-matter}"
PROJECT_DIR="${PROJECT_DIR:-/mnt/c/Users/lesli/WS/agile-smart-device}"
BUILD_DIR="${BUILD_DIR:-/opt/esp/build/agile-smart-device-matter-v3}"

EXPECTED_IDF_SHA="b774170ff46c393eeb5e495ea37936038d3f4f4f"
EXPECTED_MATTER_SHA="c91ddfbb08ccc74bb73dd6eca7422178f48b75e1"

if [[ "$(git -C "${IDF_PATH}" rev-parse HEAD)" != "${EXPECTED_IDF_SHA}" ]]; then
    echo "ESP-IDF must be pinned to ${EXPECTED_IDF_SHA}" >&2
    exit 2
fi
if [[ "$(git -C "${ESP_MATTER_PATH}" rev-parse HEAD)" != "${EXPECTED_MATTER_SHA}" ]]; then
    echo "ESP-Matter must be pinned to ${EXPECTED_MATTER_SHA}" >&2
    exit 2
fi

source "${IDF_PATH}/export.sh"
source "${ESP_MATTER_PATH}/export.sh"
mkdir -p "${BUILD_DIR}"

cd "${PROJECT_DIR}"
IDF_ARGS=(
    -B "${BUILD_DIR}"
    -DSDKCONFIG="${BUILD_DIR}/sdkconfig"
    -DSDKCONFIG_DEFAULTS="sdkconfig.defaults.matter_node"
    -DPRODUCT_PROFILE=matter_node
)
if [[ "${RHOPHI_CLAIM_DEV_BYPASS:-0}" == "1" ]]; then
    IDF_ARGS+=( -DRHOPHI_CLAIM_DEV_BYPASS=ON )
else
    IDF_ARGS+=( -DRHOPHI_CLAIM_DEV_BYPASS=OFF )
fi
if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
    idf.py "${IDF_ARGS[@]}" reconfigure build
else
    idf.py "${IDF_ARGS[@]}" set-target esp32c6 build
fi
