#!/usr/bin/env bash
set -euo pipefail

if [[ $# != 1 ]]; then
  echo 1>&2 "Usage: ./benchmark.sh path/to/hextoggle"
  exit 2
fi

HEXTOGGLE="$1"
TEMP_BIN="$(mktemp)"
TEMP_HEX="$(mktemp)"

dd if=/dev/random of="$TEMP_BIN" bs=1048576 count=64
echo -e '\n\nConvert binary -> hex:'
time "$HEXTOGGLE" "$TEMP_BIN" "$TEMP_HEX"
echo -e '\n\nConvert hex -> binary:'
time "$HEXTOGGLE" "$TEMP_HEX" "$TEMP_BIN"
rm "$TEMP_BIN" "$TEMP_HEX"
