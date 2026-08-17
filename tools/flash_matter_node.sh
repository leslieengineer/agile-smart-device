#!/usr/bin/env bash
set -euo pipefail

export IDF_PATH="${IDF_PATH:-/opt/esp/esp-idf}"
export ESP_MATTER_PATH="${ESP_MATTER_PATH:-/opt/esp/esp-matter}"
PROJECT_DIR="${PROJECT_DIR:-/mnt/c/Users/lesli/WS/agile-smart-device}"
BUILD_DIR="${BUILD_DIR:-/opt/esp/build/agile-smart-device-matter-v3}"
PORT="${PORT:-/dev/ttyS7}"

source "${IDF_PATH}/export.sh"
source "${ESP_MATTER_PATH}/export.sh"
idf.py -C "${PROJECT_DIR}" -B "${BUILD_DIR}" -p "${PORT}" flash
