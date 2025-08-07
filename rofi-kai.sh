#!/bin/bash

# Cargo las variables de entorno
source "~/.bashrc" 2> /dev/null
source "~/.profile" 2> /dev/null

# Get script directory (in case ai is local)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Ask question via Rofi
question=$(rofi -dmenu -p "Ask AI")

# Exit if cancelled
[[ -z "$question" ]] && exit 0

# Get the answer
answer="$("$SCRIPT_DIR/kai" -a "$question")"

rofi -e "$answer"
