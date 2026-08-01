#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"
CLIENT_BIN="$BUILD_DIR/client/client"
HOST="74.125.250.129"
PORT="19302"
LOGFILE="$REPO_ROOT/tests/e2e_tests/client_to_google_server.log"

if [ ! -x "$CLIENT_BIN" ]; then
  echo "Client binary not found at $CLIENT_BIN" >&2
  exit 1
fi

echo "Running client against $HOST:$PORT, logging to $LOGFILE"
"$CLIENT_BIN" "$HOST" "$PORT" >"$LOGFILE" 2>&1

GREEN="\033[0;32m"
RED="\033[0;31m"
NC="\033[0m"

if grep -Ei 'XOR-MAPPED-ADDRESS|MAPPED-ADDRESS|mapped' "$LOGFILE" >/dev/null; then
  printf "%bTEST PASSED%b\n" "$GREEN" "$NC"
  exit 0
else
  printf "%bTEST FAILED%b\n" "$RED" "$NC" >&2
  echo "No STUN mapping attribute found in log ($LOGFILE)" >&2
  exit 2
fi
