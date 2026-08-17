#!/usr/bin/env bash
set -euo pipefail

export IDF_PATH="${IDF_PATH:-/opt/esp/esp-idf}"
export ESP_MATTER_PATH="${ESP_MATTER_PATH:-/opt/esp/esp-matter}"
BUILD_DIR="${BUILD_DIR:-build-c6-thread-v16b}"

source "${IDF_PATH}/export.sh"
source "${ESP_MATTER_PATH}/export.sh"

cd "${ESP_MATTER_PATH}/examples/light"
idf.py -B "${BUILD_DIR}" \
    -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.c6_thread" \
    set-target esp32c6 build
