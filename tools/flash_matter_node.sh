#!/usr/bin/env bash
set -euo pipefail

export IDF_PATH="${IDF_PATH:-${HOME}/esp/v6.0.2/esp-idf}"
export ESP_MATTER_PATH="${ESP_MATTER_PATH:-${HOME}/esp-matter}"
PROJECT_DIR="${PROJECT_DIR:-${HOME}/WS/agile-smart-device}"
BUILD_DIR="${BUILD_DIR:-${PROJECT_DIR}/build-matter}"
PORT="${PORT:-/dev/ttyACM1}"

source "${IDF_PATH}/export.sh"
source "${ESP_MATTER_PATH}/export.sh"
idf.py -C "${PROJECT_DIR}" -B "${BUILD_DIR}" -p "${PORT}" flash
