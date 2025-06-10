#!/bin/bash

BIN="./ft_traceroute"
LOG="trace_results.log"

# Liste d'adresses à tester
DESTINATIONS=(
    "8.8.8.8"
    "1.1.1.1"
    "google.com"
    "cloudflare.com"
    "example.com"
    "doesnotexist.fake"
    "localhost"
    "123"
    "11111111"
    "34543534"
    "192.168.1.256"
    "192.168.1.1.1"
    "192.168.1.1.1"
    "123.123.123.123"
    "42342"
    "01213ddd"
    "224.0.0.1"
    "00000000001"
    "1234355654645464654"
    "192.168.001.001"
    "192.168.1"
)

echo "===== ft_traceroute test results =====" > "$LOG"
echo "Date: $(date)" >> "$LOG"
echo "" >> "$LOG"

if [[ $EUID -ne 0 ]]; then
    echo "⚠️  Le script doit être lancé en root"
    exit 1
fi

for dest in "${DESTINATIONS[@]}"; do
    echo "➤ Testing $dest ..."
    echo "===== $dest =====" >> "$LOG"
    $BIN "$dest" >> "$LOG" 2>&1
    echo "" >> "$LOG"
    sleep 1
done

echo "✅ Go -> $LOG"

