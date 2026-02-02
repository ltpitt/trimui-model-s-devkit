#!/bin/sh
set -e

# Colors for nicer output
GREEN="\033[1;32m"
BLUE="\033[1;34m"
RED="\033[1;31m"
RESET="\033[0m"

echo "${BLUE}▶ Building hellotrimui using Docker toolchain...${RESET}"

podman run --rm \
    -v "$PWD":/src \
    trimui-dev \
    sh -c '$CC $CFLAGS --static /src/hellotrimui.c $LDFLAGS -o /src/hellotrimui'

echo "${GREEN}✔ Build complete!${RESET}"
echo "${GREEN}✔ Output: ./hellotrimui${RESET}"

echo "${BLUE}▶ Killing any running hellotrimui on device...${RESET}"
adb shell killall -9 hellotrimui

echo "${BLUE}▶ Creating directory on TrimUI device...${RESET}"
adb shell mkdir -p /mnt/SDCARD/Apps/hellotrimui/

echo "${BLUE}▶ Pushing hellotrimui to TrimUI device...${RESET}"
adb push hellotrimui /mnt/SDCARD/Apps/hellotrimui/

echo "${BLUE}▶ Running hellotrimui on TrimUI device...${RESET}"
adb shell /mnt/SDCARD/Apps/hellotrimui/hellotrimui
