#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"
SERVER_BIN="$BUILD_DIR/server/server"
CLIENT_BIN="$BUILD_DIR/client/client"
HOST="127.0.0.1"
PORT="3478"
SERVER_LOG="$REPO_ROOT/tests/e2e_tests/server.log"
CLIENT_LOG="$REPO_ROOT/tests/e2e_tests/client.log"

if [ ! -x "$SERVER_BIN" ]; then
  echo "Server binary not found at $SERVER_BIN" >&2
  exit 1
fi

if [ ! -x "$CLIENT_BIN" ]; then
  echo "Client binary not found at $CLIENT_BIN" >&2
  exit 1
fi

# ensure logs dir exists
mkdir -p "$REPO_ROOT/tests/e2e_tests"


echo "Starting server: $SERVER_BIN $PORT --xor-mapped-address (logging to $SERVER_LOG)"
"$SERVER_BIN" "$PORT" --xor-mapped-address >"$SERVER_LOG" 2>&1 & server_pid=$!

trap 'kill "$server_pid" 2>/dev/null || true; wait "$server_pid" 2>/dev/null || true' EXIT
sleep 0.5

echo "Running client against $HOST:$PORT, logging to $CLIENT_LOG"
"$CLIENT_BIN" "$HOST" "$PORT" >"$CLIENT_LOG" 2>&1 || true

# give server a moment then stop it
sleep 0.2
kill "$server_pid" 2>/dev/null || true
wait "$server_pid" 2>/dev/null || true
trap - EXIT

GREEN="\033[0;32m"
RED="\033[0;31m"
NC="\033[0m"

if grep -Ei 'XOR-MAPPED-ADDRESS|MAPPED-ADDRESS|mapped' "$CLIENT_LOG" >/dev/null; then
  printf "%bTEST PASSED%b\n" "$GREEN" "$NC"
  exit 0
else
  printf "%bTEST FAILED%b\n" "$RED" "$NC" >&2
  echo "No STUN mapping attribute found in client log ($CLIENT_LOG)" >&2
  echo "Server log: $SERVER_LOG" >&2
  exit 2
fi
