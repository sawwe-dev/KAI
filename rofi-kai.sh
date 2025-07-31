#!/bin/bash

# Get the directory where the script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Ask user for a question via Rofi
question=$(rofi -dmenu -p "Ask AI:")

# Exit if no input
if [[ -z "$question" ]]; then
    exit 0
fi

# Run the AI executable from the script's directory
answer="$("$SCRIPT_DIR/kai" -a "$question")"

# Display the answer
echo "$answer" | rofi -e "$(cat)"

